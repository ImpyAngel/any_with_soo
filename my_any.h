#ifndef ANY_H
#define ANY_H

#include <typeinfo>
#include <cstdio>
#include <type_traits>
#include <algorithm>

struct any {
private:

    enum storage_state {
        EMPTY,
        SMALL,
        BIG
    };


    storage_state state;

    static const size_t SMALL_SIZE = 16;

    union storage_all {
        void *dy;
        std::aligned_storage<SMALL_SIZE, SMALL_SIZE>::type stack;
    };

    storage_all storage;

    struct storage_t {

        const std::type_info &(*type)();

        void (*destroy)(storage_all &);

        void (*copy)(const storage_all &src, storage_all &dest);

        void (*move)(storage_all &src, storage_all &dest);

        void (*swap)(storage_all &lhs, storage_all &rhs);
    };

    storage_t *data;

    template<typename T>
    struct dy_storage {
        dy_storage() {}

        static const std::type_info &type() {
            return typeid(T);
        }

        static void destroy(storage_all &storage) {
            delete reinterpret_cast<T *>(storage.dy);
        }

        static void copy(const storage_all &src, storage_all &dest) {
            dest.dy = new T(*reinterpret_cast<const T *>(src.dy));
        }

        static void move(storage_all &src, storage_all &dest) {
            dest.dy = src.dy;
            src.dy = nullptr;
        }

        static void swap(storage_all &lhs, storage_all &rhs) {
            std::swap(lhs.dy, rhs.dy);
        }
    };

    template<typename T>
    struct static_storage {
        static_storage() {}

        static const std::type_info &type() {
            return typeid(T);
        }

        static void destroy(storage_all &storage) {
            reinterpret_cast<T *>(&storage.stack)->~T();
        }

        static void copy(const storage_all &src, storage_all &dest) {
            new(&dest.stack) T(reinterpret_cast<const T &>(src.stack));
        }

        static void move(storage_all &src, storage_all &dest) {
            new(&dest.stack) T(std::move(reinterpret_cast<T &>(src.stack)));
            destroy(src);
        }

        static void swap(storage_all &lhs, storage_all &rhs) {
            std::swap(reinterpret_cast<T &>(lhs.stack), reinterpret_cast<T &>(rhs.stack));
        }
    };

    const std::type_info &type() const {
        return empty() ? typeid(void) : data->type();
    }

    template<typename T>
    T *cast() {
        switch (state) {
            case EMPTY:
                return nullptr;
            case SMALL:
                return reinterpret_cast<T *>(&storage.stack);
            case BIG:
                return reinterpret_cast<T *>(storage.dy);
        }
        return nullptr;
    }

    template<typename T>
    const T *cast() const {
        switch (state) {
            case EMPTY:
                return nullptr;
            case SMALL:
                return reinterpret_cast<const T *>(&storage.stack);
            case BIG:
                return reinterpret_cast<const T *>(storage.dy);
        }
        return nullptr;
    }


    template<typename ValueType>
    struct is_small {
        const static bool value =
                (std::is_nothrow_copy_constructible<ValueType>::value) && (sizeof(ValueType) <= SMALL_SIZE);
    };

    template<typename ValueType>
    typename std::enable_if<!is_small<ValueType>::value>::type construct(ValueType &&value) {
        using T = typename std::decay<ValueType>::type;
        static storage_t wrapper = {
                dy_storage<T>::type,
                dy_storage<T>::destroy,
                dy_storage<T>::copy,
                dy_storage<T>::move,
                dy_storage<T>::swap
        };
        data = &wrapper;
        storage.dy = new T(std::forward<ValueType>(value));
        state = BIG;
    }


    template<typename ValueType>
    typename std::enable_if<is_small<ValueType>::value>::type construct(ValueType &&value) {
        using T = typename std::decay<ValueType>::type;
        static storage_t wrapper = {
                static_storage<T>::type,
                static_storage<T>::destroy,
                static_storage<T>::copy,
                static_storage<T>::move,
                static_storage<T>::swap
        };
        data = &wrapper;
        new(&storage.dy) T(std::forward<ValueType>(value));
        state = SMALL;
    }

public:

    any() : state(EMPTY), data(nullptr) {}

    any(const any &other) : state(other.state), data(other.data) {
        // std::cout << "1\n";
        if (!other.empty()) {
            other.data->copy(other.storage, storage);
        }
    }


    any(any &&other) : state(other.state), data(other.data) {
        // std::cout << "2\n";
        if (!other.empty()) {
            other.data->move(other.storage, storage);
            other.data = nullptr;
        }
    }

    any &operator=(const any &other) {
        // std::cout << "3\n";
        any(other).swap(*this);
        return *this;
    }

    any &operator=(any &&other) {
        // std::cout << "4\n";
        any(std::move(other)).swap(*this);
        return *this;
    }

    template<typename T,
    typename = typename std::enable_if<!std::is_same<any, std::decay_t<T>>::value>::type
    > 
    any(T &&value) {
        // std::cout << "5\n";
        construct(std::forward<T>(value));
    }

    template<typename ValueType>
    any &operator=(ValueType &&value) {
        // std::cout << "6\n";  
        any(std::forward<ValueType>(value)).swap(*this);
        return *this;
    }

    ~any() {
        if (!empty()) {
            data->destroy(storage);
            data = nullptr;
            state = EMPTY;
        }
    }

    void swap(any &other) {     
        if (data != other.data) {
            any tmp(std::move(other));
            other.data = data;
            other.state = state;
            if (state != EMPTY) {
                data->move(storage, other.storage);
            }
            data = tmp.data;
            state = tmp.state;
            if (tmp.data != nullptr) {
                tmp.data->move(tmp.storage, storage);
                tmp.data = nullptr;
                tmp.state = EMPTY;
            }
        } else {
            if (state != EMPTY) {
                data->swap(storage, other.storage);
            }
}
    }

    bool empty() const {
        return state == EMPTY;
    }

    template<typename T>
    friend const T* any_cast(const any *operand);

    template<typename T>
    friend T* any_cast(any * operand);

    template <typename VT>
    friend VT any_cast(const any& operand);

    template <typename VT>
    friend VT any_cast(any& operand);

    template <typename VT>
    friend VT any_cast(any&& operand);
};

    struct  bad_any_cast :public std::exception{

};


template <typename VT>
    VT any_cast(const any& operand) {
    // if (operand.type() != typeid(VT))
    //     throw new bad_any_cast();
    // else
        return *operand.cast<VT>();
}

template <typename VT>
    VT any_cast(any& operand) {
    // if (operand.type() != typeid(VT))
    //     throw new bad_any_cast();
    // else
        return *operand.cast<VT>();
}

template <typename VT>
    VT any_cast(any&& operand) {
    // if (operand.type() != typeid(VT))
    //     throw new bad_any_cast();
    // else
        return *operand.cast<VT>();
}

template<typename T>
const T *any_cast(const any *operand) {
    // if (operand == nullptr || operand->type() != typeid(T))
    //     throw new bad_any_cast();
    // else
        return operand->cast<T>();
}

template<typename T>
T *any_cast(any *operand) {
    // if (operand == nullptr || operand->type() != typeid(T))
    //     throw new bad_any_cast();
    // else
        return operand->cast<T>();
}


#endif
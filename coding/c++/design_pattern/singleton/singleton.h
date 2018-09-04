#include <iostream>

class singleton
{
    private:
        int data;
        singleton():data(0) {}                    // Constructor (the {} brackets) are needed here.

        // C++ 03
        // ========
        // Don't forget to declare these two. You want to make sure they
        // are unacceptable otherwise you may accidentally get copies of
        // your singleton appearing.
        singleton(singleton const&)       = delete;
        void operator=(singleton const&)  = delete;
        ~singleton() {}                        // destructor is private

        // C++ 11
        // =======
        // We can use the better technique of deleting the methods
        // we don't want.
    public:
        static singleton& getInstance()
        {
            static singleton    instance;   // Guaranteed to be destroyed. Instantiated on first use.
            return instance;
        }

        int getData() {
            return this -> data;
        }

        void setData(int data) {
            this -> data = data;
        }
};

#include "singleton.h"

int main()
{
    singleton& s = s.getInstance();
    std::cout << "Initial: " << s.getData() << std::endl;
    s.setData(100);
    std::cout << "After:   " << s.getData() << std::endl;
    return 0;
}

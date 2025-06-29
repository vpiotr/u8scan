#include "module1/module1.h"
#include "module2/module2.h"
#include <iostream>

int main() {
    std::cout << "U8SCAN Multi-Module Demo\n";
    std::cout << "=====================\n\n";
    
    // Run tests from both modules
    std::cout << "Running tests from Module 1...\n";
    bool module1_success = module1_run_test();
    
    std::cout << "\nRunning tests from Module 2...\n";
    bool module2_success = module2_run_test();
    
    // Display UTF-8 processing results from both modules
    std::cout << "\nModule 1 Processing:\n";
    std::cout << "-------------------\n";
    std::cout << module1_process_utf8();
    
    std::cout << "\nModule 2 Processing:\n";
    std::cout << "-------------------\n";
    std::cout << module2_process_complex_utf8();
    
    // Overall success
    if (module1_success && module2_success) {
        std::cout << "\nAll tests passed successfully!\n";
        return 0;
    } else {
        std::cout << "\nSome tests failed!\n";
        return 1;
    }
}

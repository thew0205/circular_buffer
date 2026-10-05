#include "CppUTest/TestHarness.h"
#include "CppUTest/CommandLineTestRunner.h"


int main(){

    const char *fav[] = {"sunspec", "-c"};
    return CommandLineTestRunner::RunAllTests(sizeof(fav) / sizeof(fav[0]), fav);
    return 0;
}
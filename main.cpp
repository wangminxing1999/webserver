#include"server.h"
int main() {
    server sv(1315, 60000, 4);
    sv.start();
}

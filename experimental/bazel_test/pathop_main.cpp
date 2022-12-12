
#include "include/core/SkPath.h"
#include "include/pathops/SkPathOps.h"

#include <cstdio>

int main(int argc, char** argv) {
    SkPath path1;
    path1.moveTo(10, 10);
    path1.lineTo(15, 5);
    path1.lineTo(20, 10);
    path1.close();

    SkPath path2;
    path2.moveTo(12, 12);
    path2.lineTo(18, 6);
    path2.lineTo(24, 12);
    path2.close();

    SkPath combined;
    if Op(path1, path2, kIntersect_SkPathOp, &combined) {
        printf("Success: \n")
        combined.dump();
        printf("\n");
    } else {
        printf("Operation failed\n");
    }
}

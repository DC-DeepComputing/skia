#ifndef SkCubics_DEFINED
#define SkCubics_DEFINED

class SkCubics {
public:
    static int RootsValidT(double A, double B, double C, double D,
                           double solution[3]);
    static int RootsReal(double A, double B, double C, double D,
                         double solution[3]);
};

#endif

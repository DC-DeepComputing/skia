
int fibonacci_ii(int n) {
    return n <= 1 ? n : fibonacci_ii(n - 1) + fibonacci_ii(n - 2);
}

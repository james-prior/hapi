// Create mask that has m one bits followed by n zero bits.
#define MASK(m, n) (((1<<(m)) - 1) << (n))

#define FAHRENHEIT(celsius) ((celsius) * 9.0 / 5.0 + 32.0)

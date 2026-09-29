
struct numbers {
    int a;
    int b;
    char op;
};

/*se status: 0 = ok, 1 = divisao por zero, 2 = operacao invalida */
struct resultado {
    int status;
    int valor;
};

program CALC_PROG {
    version CALC_VERS {
        resultado calc(numbers) = 1;
    } = 1;
} = 0x23451111;

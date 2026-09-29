#include "calc_ops.h"

int calcular(int a, int b, char op, int *valor)
{
    *valor = 0;
    switch (op) {
        case '+': *valor = a + b; return ST_OK;
        case '-': *valor = a - b; return ST_OK;
        case '*': *valor = a * b; return ST_OK;
        case '/':
            if (b == 0)
                return ST_DIV_ZERO;
            *valor = a / b;
            return ST_OK;
        default:
            return ST_OP_INVALIDA;
    }
}

const char *status_msg(int status)
{
    switch (status) {
        case ST_OK:           return "Ok";
        case ST_DIV_ZERO:     return "Divisao por zero";
        case ST_OP_INVALIDA:  return "Operacao invalida";
        default:              return "Erro desconhecido";
    }
}

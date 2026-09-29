#ifndef CALC_OPS_H
#define CALC_OPS_H

#define ST_OK        0
#define ST_DIV_ZERO  1
#define ST_OP_INVALIDA 2

int calcular(int a, int b, char op, int *valor);

const char *status_msg(int status);

#endif

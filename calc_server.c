#include "calc.h"
#include "calc_ops.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>

struct tarefa {
    numbers   entrada;
    resultado saida;
    char      cliente[64];
};


static void *trabalhador(void *arg)
{
    struct tarefa *t = (struct tarefa *)arg;

    t->saida.status = calcular(t->entrada.a, t->entrada.b,
                               t->entrada.op, &t->saida.valor);

    if (t->saida.status == ST_OK)
        printf("%s -> %d %c %d = %d\n", t->cliente, t->entrada.a,
               t->entrada.op, t->entrada.b, t->saida.valor);
    else
        printf("%s -> %d %c %d = ERRO (%s)\n", t->cliente, t->entrada.a,
               t->entrada.op, t->entrada.b, status_msg(t->saida.status));
    fflush(stdout);

    return NULL;
}

/* Identifica o cliente (IP) que fez a chamada, para exibir no log. */
static void nome_cliente(struct svc_req *rqstp, char *buf, size_t tam)
{
    struct netbuf *nb = svc_getrpccaller(rqstp->rq_xprt);
    char host[NI_MAXHOST] = "?";

    if (nb && nb->buf)
        getnameinfo((struct sockaddr *)nb->buf, nb->len, host, sizeof host,
                    NULL, 0, NI_NUMERICHOST);
    snprintf(buf, tam, "%s", host);
}

bool_t calc_1_svc(numbers *argp, resultado *result, struct svc_req *rqstp)
{
    pthread_t th;
    struct tarefa t;

    t.entrada = *argp;
    t.saida.status = ST_OP_INVALIDA;
    t.saida.valor = 0;
    nome_cliente(rqstp, t.cliente, sizeof t.cliente);

    if (pthread_create(&th, NULL, trabalhador, &t) != 0)
        trabalhador(&t);
    else
        pthread_join(th, NULL);

    *result = t.saida;
    return TRUE;
}

int calc_prog_1_freeresult(SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)
{
    (void)transp;
    xdr_free(xdr_result, result);
    return 1;
}

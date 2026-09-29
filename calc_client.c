#include "calc.h"
#include "calc_ops.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_OPS 256

struct job {
    int         indice;
    int         a, b;
    char        op;
    const char *host;
    int         rpc_ok;
    int         status;
    int         valor;
    char        erro[128];
};

static void executa_remoto(struct job *j)
{
    CLIENT *clnt;
    numbers arg = { j->a, j->b, j->op };
    resultado res;
    struct timeval to = { 5, 0 };

    clnt = clnt_create(j->host, CALC_PROG, CALC_VERS, "tcp");
    if (clnt == NULL) {
        snprintf(j->erro, sizeof j->erro, "%s", clnt_spcreateerror(j->host));
        return;
    }
    clnt_control(clnt, CLSET_TIMEOUT, (char *)&to);

    memset(&res, 0, sizeof res);
    if (calc_1(&arg, &res, clnt) != RPC_SUCCESS) {
        snprintf(j->erro, sizeof j->erro, "%s", clnt_sperror(clnt, "chamada falhou"));
    } else {
        j->rpc_ok = 1;
        j->status = res.status;
        j->valor  = res.valor;
    }
    clnt_destroy(clnt);
}

static void *trabalhador(void *arg)
{
    struct job *j = (struct job *)arg;

    printf("Sou a thread %d: %d %c %d (%s)\n", j->indice,
           j->a, j->op, j->b, j->host ? "remoto" : "local");

    if (j->host) {
        executa_remoto(j);
    } else {
        j->status = calcular(j->a, j->b, j->op, &j->valor);
        j->rpc_ok = 1;
    }
    return NULL;
}

static int le_operacao(const char *linha, struct job *j)
{
    int a, b;
    char op;
    if (sscanf(linha, " %d %c %d", &a, &op, &b) != 3)
        return 0;
    j->a = a; j->b = b; j->op = op;
    return 1;
}

int main(int argc, char *argv[])
{
    struct job jobs[MAX_OPS];
    pthread_t threads[MAX_OPS];
    int n = 0, opt;
    const char *arquivo = NULL, *destino;
    char linha[256];
    FILE *in;

    while ((opt = getopt(argc, argv, "f:")) != -1) {
        if (opt == 'f') arquivo = optarg;
        else goto uso;
    }
    if (optind >= argc) {
uso:
        fprintf(stderr, "Uso: %s [-f arquivo] <host|local>\n", argv[0]);
        return 1;
    }
    destino = argv[optind];

    if (arquivo) {
        in = fopen(arquivo, "r");
        if (!in) { perror(arquivo); return 1; }
    } else {
        in = stdin;
        printf("=== CALCULADORA RPC (threads) - destino: %s ===\n", destino);
        printf("Digite operacoes no formato \"10 + 5\" (+ - * /).\n");
        printf("Linha vazia ou \"run\" executa todas em paralelo.\n");
    }

    while (n < MAX_OPS) {
        if (!arquivo) { printf("op %d> ", n + 1); fflush(stdout); }
        if (!fgets(linha, sizeof linha, in)) break;
        if (linha[0] == '#') continue;
        if (linha[0] == '\n' || strncmp(linha, "run", 3) == 0) {
            if (arquivo) continue;
            break;
        }
        if (!le_operacao(linha, &jobs[n])) {
            fprintf(stderr, "Linha ignorada (formato invalido): %s", linha);
            continue;
        }
        jobs[n].indice = n + 1;
        jobs[n].host = strcmp(destino, "local") == 0 ? NULL : destino;
        jobs[n].rpc_ok = 0;
        jobs[n].status = ST_OK;
        jobs[n].valor = 0;
        jobs[n].erro[0] = '\0';
        n++;
    }
    if (arquivo) fclose(in);

    if (n == 0) { printf("Nenhuma operacao informada.\n"); return 0; }

    printf("\nExecutando %d operacao(oes) em %d thread(s)...\n\n", n, n);

    for (int i = 0; i < n; i++) {
        if (pthread_create(&threads[i], NULL, trabalhador, &jobs[i]) != 0) {
            fprintf(stderr, "Erro ao criar a thread %d\n", i + 1);
            return 1;
        }
    }
    for (int i = 0; i < n; i++)
        pthread_join(threads[i], NULL);

    printf("\n=== Resultados ===\n");
    for (int i = 0; i < n; i++) {
        struct job *j = &jobs[i];
        printf("%d) %d %c %d = ", j->indice, j->a, j->op, j->b);
        if (!j->rpc_ok)
            printf("FALHA RPC: %s\n", j->erro);
        else if (j->status != ST_OK)
            printf("ERRO (%s)\n", status_msg(j->status));
        else
            printf("%d\n", j->valor);
    }
    return 0;
}

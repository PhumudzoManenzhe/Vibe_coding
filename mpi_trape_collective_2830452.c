#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

double f(double x) {
    return 4.0 / (1.0 + x * x);
}

double Trap(double left_endpt, double right_endpt, int trap_count, double base_len) {
    double estimate, x;
    int i;
    estimate = (f(left_endpt) + f(right_endpt)) / 2.0;
    for (i = 1; i <= trap_count - 1; i++) {
        x = left_endpt + i * base_len;
        estimate += f(x);
    }
    return estimate * base_len;
}

int main(int argc, char** argv) {
    int my_rank, comm_sz, n, local_n;
    double a, b, h, local_a, local_b;
    double local_int, total_int;
    double t_start, t_end, elapsed_seconds;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    if (argc != 4) {
        if (my_rank == 0) fprintf(stderr, "Usage: %s <a> <b> <n>\n", argv[0]);
        MPI_Finalize();
        exit(1);
    }

    if (my_rank == 0) {
        a = atof(argv[1]);
        b = atof(argv[2]);
        n = atoi(argv[3]);
    }

    MPI_Bcast(&a, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&b, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    h = (b - a) / n;
    local_n = n / comm_sz;
    local_a = a + my_rank * local_n * h;
    local_b = local_a + local_n * h;

    t_start = MPI_Wtime();

    local_int = Trap(local_a, local_b, local_n, h);

    MPI_Reduce(&local_int, &total_int, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    t_end = MPI_Wtime();
    elapsed_seconds = t_end - t_start;

    if (my_rank == 0) {
        printf("With n=%d trapezoids, estimate of the integral from %lf to %lf = %.15e in %.6fs\n",
               n, a, b, total_int, elapsed_seconds);
    }

    MPI_Finalize();
    return 0;
}
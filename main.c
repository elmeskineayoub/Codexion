#include "codexion.h"

int main(int argc, char **argv)
{
    int values[6];
    int dongle_cooldown;
    int i;

    if (argc != 9)
    {
        print_error("[ERROR] wrong number of arguments!\n");
        exit(1);
    }
    i = 1;
    while (i <= 6)
    {
        if (!is_number(argv[i]))
        {
            print_error("[ERROR] invalid numeric argument!\n");
            exit(1);
        }
        values[i - 1] = ft_atoi(argv[i]);
        if (values[i - 1] <= 0)
        {
            print_error("[ERROR] arguments must be positive!\n");
            exit(1);
        }
        i++;
    }
    if (!is_number(argv[i]))
    {
        print_error("[ERROR] invalid numeric argument!\n");
        exit(1);
    }
    dongle_cooldown = ft_atoi(argv[i]);
    if (dongle_cooldown < 0)
    {
        print_error("[ERROR] cooldown must be non-negative!\n");
        exit(1);
    }
    if (!valid_scheduler(argv[8]))
    {
        print_error("[ERROR] scheduler must be fifo or edf!\n");
        exit(1);
    }
    return 0;
}
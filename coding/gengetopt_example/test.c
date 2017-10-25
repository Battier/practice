#include <stdio.h>
#include <stdlib.h>
#include "cmdline.h"

void perform_stuff(const char *input_filename, const char *output_filename) {
    printf("Reading from input file %s\n", input_filename);
    printf("Writing to output file %s\n", output_filename);
}

int main(int argc, char *argv[]) {
    struct gengetopt_args_info args_info;
    char *output_filename = "Hello World";
    if (cmdline_parser(argc, argv, &args_info) != 0) {
        exit(1);
    }

    if (args_info.inputs_num != 1) {
        fprintf(stderr, "Program requires filename argument\n");
        return EXIT_FAILURE;
    }
    const char *input_filename = args_info.inputs[0];

    perform_stuff(input_filename, output_filename);

    return EXIT_SUCCESS;
}

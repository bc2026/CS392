int main(int argc, char const *argv[])
{
    char *a = "Usage: $0 [-f question_file] [-i IP_address] [-p port_number] [-h]\n\
              - f question_file\tDefault to 'question.txt';\n\
              - i IP_address\tDefault to '127.0.0.1';\n\
              - p port_number\tDefault to 25555;\n\
              - h            \tDisplay this help info.";

    printf("%d\n", sizeof(a));
    return 0;
}

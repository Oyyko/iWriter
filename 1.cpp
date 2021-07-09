#include "markdown.hpp"
#include <fstream>
class Options
{
private:
    std::string input_file_name;

public:
    bool read_options(int argc, char *argv[]);
    void show_help();
    std::string get_input_file_name() { return input_file_name; }
};

bool Options::read_options(int argc, char *argv[])
{
    if (argc == 1)
    {
        std::cerr << "Please input file name!" << std::endl;
        show_help();
    }
    bool help{false}, error{false};
    for (int i = 1; i < argc && !help && !error; ++i)
    {
        if (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] != 0)
        {
            std::string option(argv[i] + 2);
            if (option == "help")
            {
                help = true;
            }
            else if (option == "set")
            {
                std::string username(argv[i + 1]);
                std::string password(argv[i + 2]);
                i = i + 2;
                string script = "ftp -n <<- EOF\n\n"
                                "open home.ustc.edu.cn\n\n"
                                "user " +
                                username + " " + password + "\n\n"
                                                            "binary\n\n"
                                                            "cd /public_html\n\n"
                                                            "put index.html\n\n"
                                                            "put github-markdown.css\n\n"
                                                            "bye\n\n"
                                                            "EOF\n";
                ofstream of("upload.sh", fstream::out);
                of << script;
                of.close();
            }
            else
            {
                error = true;
                std::cerr << "Unknown option " << argv[i] << "\nuse --help for help." << std::endl;
            }
        }
        else
        {
            if (input_file_name.empty())
            {
                input_file_name = argv[i];
            }
            else
            {
                error = true;
                std::cerr << "Too many parameters.\nUse --help for help." << std::endl;
            }
        }
    }
    if (help)
    {
        show_help();
        return false;
    }
    return !error;
}

void Options::show_help()
{
    const char *help_text =
        "Usage:\n"
        "    trans [<option>...] [input-file]\n"
        "\n"
        "Available options are:\n"
        "    --help      Show help.\n"
        "    --set aaa bbb \n"
        "               Set your username as aaa, password as bbb\n";
    std::cerr
        << std::endl
        << help_text << std::endl;
}

int main(int argc, char *argv[])
{
    Options opt;
    if (!opt.read_options(argc, argv))
    {
        return EXIT_FAILURE;
    }
    if (!opt.get_input_file_name().empty())
    {
        std::cerr << "Reading file '" << opt.get_input_file_name() << "'..." << std::endl;
        std::ifstream fs(opt.get_input_file_name());
        if (!fs.is_open())
        {
            std::cerr << "ERROR: fail to open file!\n";
            return EXIT_FAILURE;
        }
        else
        {
            std::cerr << "Reading successfully!\n";
        }
        Document doc(fs);
        string catalog{doc.get_catalog()};
        string html_text{doc.get_html_text()};
        ofstream out;
        out.open("index.html", fstream::out);
        std::string head = "<!DOCTYPE html><html><head>\
        <meta charset=\"utf-8\">\
        <title>Markdown</title>\
        <link rel=\"stylesheet\" href=\"github-markdown.css\">\
        </head><body><article class=\"markdown-body\">";
        std::string end = "</article></body></html>";
        out << head + catalog + html_text + end;
        out.close();
        system("./upload.sh");
        cout << "Sucessfully executed!\n";
        return 0;
    }
}
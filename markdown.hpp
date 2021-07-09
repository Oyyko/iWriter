#ifndef MARKDOWN_H_INCLUDED
#define MARKDOWN_H_INCLUDED

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cstring>

using std::cout;
using std::string;
using std::vector;
using namespace std;

const int max_line_length = 123456;

enum token
{
    h1,           //0
    h2,           //1
    h3,           //2
    h4,           //3
    h5,           //4
    h6,           //5
    p,            //6
    hyper_link,   //7
    ul,           //8
    ol,           //9
    li,           //10
    code_block,   //11
    code_in_line, //12
    quote,        //13
    image,        //14
    em,           //15
    strong,       //16
    hr,           //17
    br,           //18
    nul,          //19
    em_strong     //20
};

struct space_num_and_real_location
{
    int space_num;
    char *real_location;
    space_num_and_real_location(int a, char *b)
    {
        space_num = a;
        real_location = b;
    }
};

struct type_and_real_loc
{
    int type;
    char *real_loc;
    type_and_real_loc(int a, char *b)
    {
        type = a;
        real_loc = b;
    }
};

const string front_tag[] = {"<h1", "<h2", "<h3", "<h4", "<h5", "<h6", "<p>", "", "<ul>", "<ol>", "<li>", "<pre><code>", "<code>", "<blockquote", "", "<em>", "<strong>", "<hr color=#CCCCCC size=1 />", "<br />", ""};

const string back_tag[] = {"</h1>", "</h2>", "</h3>", "</h4>", "</h5>", "</h6>", "</p>", "", "</ul>", "</ol>", "</li>", "</code></pre>", "</code>", "</blockquote>", "", "</em>", "</strong>", "", "", ""};

struct catalog_node
{
    vector<catalog_node *> child;
    string heading;
    string tag;
    catalog_node(const string &hd) : heading(hd) {}
};

struct node
{
    int type;
    vector<node *> child;
    string text;
    string title;
    string link;
    node(int t) : type(t) {}
};

class Document
{
private:
    string html_text;
    node *root;
    catalog_node *c_root;
    string catalog;
    node *now;
    char line[max_line_length];

    bool is_heading(node *x)
    {
        return x->type >= h1 && x->type <= h6;
    }
    bool is_image(node *x)
    {
        return x->type == image;
    }
    bool is_href(node *x)
    {
        return x->type == hyper_link;
    }

    template <typename N>
    void destroy(N *x)
    {
        for (auto m : x->child)
        {
            destroy(m);
        }
        delete x;
    }

    void c_dfs(catalog_node *x, string index)
    {
        catalog += "<li>\n";
        catalog += "<a href=\"#" + x->tag + "\">" + index + " " + x->heading + "</a>\n";
        int n = static_cast<int>(x->child.size());
        if (n)
        {
            catalog += "<ul>\n";
            for (int i = 0; i < n; ++i)
            {
                c_dfs(x->child[i], index + to_string(i + 1) + ".");
            }
            catalog += "</ul>\n";
        }
        catalog += "</li>\n";
    }

    void c_insert(catalog_node *v, int depth, const string &hd, int tag)
    {
        if (depth == 1)
        {
            v->child.push_back(new catalog_node(hd));
            v->child.back()->tag = "tag" + to_string(tag);
            return;
        }
        if (v->child.size() == 0 || v->child.back()->heading.empty())
        {
            v->child.push_back(new catalog_node(""));
        }
        c_insert(v->child.back(), depth - 1, hd, tag);
    }

    void dfs(node *v)
    {
        if (v->type == p && v->text.empty() && v->child.empty())
            return;
        html_text += front_tag[v->type];
        bool flag = true;

        //deal heading
        if (is_heading(v))
        {
            html_text += "id=\"" + v->text + "\">";
            flag = false;
        }

        //deal hyper link
        if (is_href(v))
        {
            html_text += "<a href=\"" + v->link + "\" title=\"" + v->title + "\">" + v->text + "</a>";
            flag = false;
        }

        //deal image
        if (is_image(v))
        {
            html_text += "<img alt=\"" + v->text + "\" src=\"" + v->link + "\" title=\"" + v->title + "\" />";
            flag = false;
        }

        if (flag)
        {
            html_text += v->text;
            flag = false;
        }

        for (auto xx : v->child)
        {
            dfs(xx);
        }

        html_text += back_tag[v->type];
    }

    bool is_next_line(char *s)
    {
        int cnt = 0;
        char *p = s;
        while (*p)
        {
            if (*p != ' ' && *p != '\t' && *p != '-')
            {
                return false;
            }
            if (*p == '-')
            {
                cnt++;
            }
            p++;
        }
        return cnt >= 3; // ---为分割线
    }

    void make_paragraph(node *v)
    {
        if (v->child.size() == 1u && v->child.back()->type == p)
        {
            return;
        }
        if (v->type == p)
        {
            return;
        }
        if (v->type == nul)
        {
            v->type = p;
            return;
        }
        node *x = new node(p);
        x->child = v->child;
        v->child.clear();
        v->child.push_back(x);
    }

    space_num_and_real_location line_start(char *s)
    {
        if (static_cast<int>(strlen(s)) == 0)
        {
            return space_num_and_real_location(0, nullptr);
        }

        int space_num{};
        int tab_num{};
        for (int i = 0; s[i] != '\0'; ++i)
        {
            if (s[i] == ' ')
            {
                space_num++;
            }
            else if (s[i] == '\t')
            {
                tab_num++;
            }
            else
            {
                return space_num_and_real_location(tab_num * 4 + space_num, s + i);
            }
        }
        return space_num_and_real_location(0, nullptr);
    }
    type_and_real_loc analyse_type(char *s)
    {
        char *p = s;
        for (; *p == '#'; p++)
            ;
        if (p - s > 0 && *p == ' ')
        {
            if (p - s <= 6) //NOTE: 7个#并不是标题
            {
                return type_and_real_loc(p - s + h1 - 1, p + 1);
            }
        }
        p = s;
        if (strncmp(p, "```", 3) == 0)
        {
            return type_and_real_loc(code_block, p + 3);
        }

        if (strncmp(p, "- ", 2) == 0)
        {
            return type_and_real_loc(ul, p + 1);
        }

        if (strncmp(p, "> ", 2) == 0)
        {
            return type_and_real_loc(quote, p + 1);
        }

        char *p1 = p;
        for (; *p1 && (isdigit(*p1)); p1++)
            ;
        if (p1 != p && p1[0] == '.' && p1[1] == ' ')
        {
            return type_and_real_loc(ol, p1 + 1);
        }
        return type_and_real_loc(6, p);
    }

    node *find_node_by_depth(int depth)
    {
        node *pp = root;
        while (!pp->child.empty() && depth)
        {
            pp = pp->child.back();
            if (pp->type == li)
            {
                depth--;
            }
        }
        return pp;
    }

    void insert(node *v, const string &str)
    {
        int n = static_cast<int>(str.size());
        bool in_code{false};
        bool in_em{false};
        bool in_strong{false};
        bool in_em_strong{false};
        v->child.push_back(new node(nul));
        char c;
        for (int i = 0; i < n; ++i)
        {
            c = str[i];
            if (c == '\\')
            {
                c = str[++i];
                v->child.back()->text += string(1, '\\');
                continue;
            }

            if (c == '`')
            {
                if (in_code)
                {
                    v->child.push_back(new node(nul));
                }
                else
                {
                    v->child.push_back(new node(code_in_line));
                }
                in_code = !in_code;
                continue;
            }

            if (c == '*')
            {
                if (!in_code)
                {
                    if (i < n - 2 && str[i + 1] == '*' && str[i + 2] == '*')
                    {
                        i += 2;
                        if (in_em_strong)
                        {
                            v->child.push_back(new node(nul));
                        }
                        else
                        {
                            v->child.push_back(new node(em_strong));
                        }
                    }
                }
            }
        }
    }

public:
    Document(std::ifstream &ifs);
    string get_html_text()
    {
        return html_text;
    }
};

Document::Document(std::ifstream &ifs)
{
    while (!ifs.eof())
    {
        ifs.getline(line, max_line_length);
        cout << line << std::endl;
    }
}

#endif
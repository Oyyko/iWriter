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
    none,         //19
    em_strong     //20
};

struct tab_num_and_real_location
{
    int tab_num;
    char *real_location;
    tab_num_and_real_location(int a, char *b)
    {
        tab_num = a;
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

const string front_tag[] = {"<h1 ", "<h2 ", "<h3 ", "<h4 ", "<h5 ", "<h6 ", "<p>", "", "<ul>", "<ol>", "<li>", "<pre><code>", "<code>", "<blockquote>", "", "<em>", "<strong>", "<hr color=#CCCCCC size=1 />", "<br />", "", "<em><strong>"};

const string back_tag[] = {"</h1>", "</h2>", "</h3>", "</h4>", "</h5>", "</h6>", "</p>", "", "</ul>", "</ol>", "</li>", "</code></pre>", "</code>", "</blockquote>", "", "</em>", "</strong>", "", "", "", "</strong></em>"};

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
    int tag_cnt{};
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

    bool is_cut_line(char *s)
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
        if (v->type == none)
        {
            v->type = p;
            return;
        }
        node *x = new node(p);
        x->child = v->child;
        v->child.clear();
        v->child.push_back(x);
    }

    tab_num_and_real_location line_start(char *s)
    {
        if (static_cast<int>(strlen(s)) == 0)
        {
            return tab_num_and_real_location(0, nullptr);
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
                return tab_num_and_real_location(tab_num + space_num / 4, s + i);
            }
        }
        return tab_num_and_real_location(0, nullptr);
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
        if (strncmp(p, "* ", 2) == 0)
        {
            return type_and_real_loc(ul, p + 1);
        }

        if (*p == '>' && p[1] == ' ')
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
        v->child.push_back(new node(none));
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
                    v->child.push_back(new node(none));
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
                            v->child.push_back(new node(none));
                        }
                        else
                        {
                            v->child.push_back(new node(em_strong));
                        }
                        in_em_strong = !in_em_strong;
                        continue;
                    }
                    if (i < n - 1 && str[i + 1] == '*')
                    {
                        i += 1;
                        if (in_strong)
                        {
                            v->child.push_back(new node(none));
                        }
                        else
                        {
                            v->child.push_back(new node(strong));
                        }
                        in_strong = !in_strong;
                        continue;
                    }

                    if (in_em)
                    {
                        v->child.push_back(new node(none));
                    }
                    else
                    {
                        v->child.push_back(new node(em));
                    }
                    in_em = !in_em;
                    continue;
                }
            }

            //image

            // ![alt](src title)
            // ![alt][src "title"]
            int temp;
            bool ff;
            if (c == '!' &&
                (i < n - 1 && str[i + 1] == '[') &&
                (!in_code) &&
                (!in_strong) &&
                (!in_em) &&
                (!in_em_strong))
            {
                temp = i; //str[i]=='!'
                ff = false;
                for (; temp + 1 < n && str[temp] != ']'; temp++)
                    ;
                //此时str[temp]==']'或者temp==n-1;
                if (temp == n - 1)
                {
                }
                else
                {
                    ff = str[temp + 1] == '(';
                    for (; temp + 1 < n && str[temp] != ')'; ++temp)
                        ;
                    ff = ff && (str[temp] == ')');
                }

                if (ff)
                {

                    v->child.push_back(new node(image));
                    for (i += 2; i < n - 1 && str[i] != ']'; ++i)
                    {
                        v->child.back()->text += string(1, str[i]);
                    }
                    //此时 s[i]指向']'
                    i++;
                    //此时 s[i]=='('
                    for (i++; i < n - 1 && str[i] != ' ' && str[i] != ')'; ++i)
                    {
                        v->child.back()->link += string(1, str[i]);
                    }
                    if (str[i] != ')')
                    {
                        for (i++; i < n - 1 && str[i] != ')'; ++i)
                        {
                            if (str[i] != '"')
                            {
                                v->child.back()->title += string(1, str[i]);
                            }
                        }
                    }
                    v->child.push_back(new node(none));
                    continue;
                }
            }

            //hyper link
            if (c == '[' &&
                !in_code &&
                !in_strong &&
                !in_em &&
                !in_em_strong)
            {
                temp = i; //str[i]=='!'
                ff = false;
                for (; temp + 1 < n && str[temp] != ']'; temp++)
                    ;
                //此时str[temp]==']'或者temp==n-1;
                if (temp == n - 1)
                {
                }
                else
                {
                    ff = str[temp + 1] == '(';
                    for (; temp + 1 < n && str[temp] != ')'; ++temp)
                        ;
                    ff = ff && (str[temp] == ')');
                }
                if (ff)
                {
                    v->child.push_back(new node(hyper_link));
                    for (i++; i < n - 1 && str[i] != ']'; ++i)
                    {
                        v->child.back()->text += string(1, str[i]);
                    }
                    i++;
                    for (i++; i < n - 1 && str[i] != ' ' && str[i] != ')'; ++i)
                    {
                        v->child.back()->link += string(1, str[i]);
                    }
                    if (str[i] != ')')
                    {
                        for (i++; i < n - 1 && str[i] != ')'; ++i)
                        {
                            if (str[i] != '"')
                            {
                                v->child.back()->title += string(1, str[i]);
                            }
                        }
                    }
                    v->child.push_back(new node(none));
                    continue;
                }
            }
            v->child.back()->text += string(1, c);
            
        }
    }

public:
    Document(std::ifstream &ifs);
    string get_html_text()
    {
        return html_text;
    }
    string get_catalog()
    {
        return catalog;
    }
    ~Document()
    {
        destroy<node>(root);
        destroy<catalog_node>(c_root);
    }
};

Document::Document(std::ifstream &ifs)
{
    root = new node(none);
    c_root = new catalog_node("");
    now = root;
    bool new_p{false};
    bool in_code_block{false};
    while (!ifs.eof())
    {
        ifs.getline(line, max_line_length);
        if (!in_code_block && is_cut_line(line))
        {
            now = root;
            now->child.push_back(new node(hr));
            new_p = false;
            continue;
        }

        tab_num_and_real_location tnarl = line_start(line);

        if (!in_code_block && tnarl.real_location == nullptr)
        {
            now = root;
            new_p = true;
            continue;
        }

        type_and_real_loc tarl = analyse_type(tnarl.real_location);

        if (tarl.type == code_block)
        {
            if (in_code_block)
            {
                now->child.push_back(new node(none));
            }
            else
            {
                now->child.push_back(new node(code_block));
            }
            in_code_block = !in_code_block;
            continue;
        }

        if (in_code_block)
        {
            now->child.back()->text += string(line) + '\n';
            continue;
        }

        if (tarl.type == p)
        {
            if (now == root)
            {
                now = find_node_by_depth(tnarl.tab_num);

                now->child.push_back(new node(p));

                now = now->child.back();
            }
            bool flag{false};
            if (new_p && !now->child.empty())
            {
                node *pp = nullptr;
                for (auto x : now->child)
                {
                    if (x->type == none)
                    {
                        pp = x;
                    }
                }
                if (pp != nullptr)
                {
                    make_paragraph(pp);
                }
                flag = true;
            }
            if (flag)
            {
                now->child.push_back(new node(p));
                now = now->child.back();
            }
            now->child.push_back(new node(none));
            insert(now->child.back(), string(tarl.real_loc));
            new_p = false;
            continue;
        }

        now = find_node_by_depth(tnarl.tab_num);

        //heading
        if (tarl.type >= h1 && tarl.type <= h6)
        {
            now->child.push_back(new node(tarl.type));
            now->child.back()->text = "tag" + to_string(++tag_cnt);
            insert(now->child.back(), string(tarl.real_loc));
            c_insert(c_root, tarl.type - h1 + 1, string(tarl.real_loc), tag_cnt);
        }

        //unordered list
        if (tarl.type == ul)
        {
            if (now->child.empty() || now->child.back()->type != ul)
            {
                now->child.push_back(new node(ul));
            }
            now = now->child.back();
            bool flag = false;
            if (new_p && !now->child.empty())
            {
                node *ptr = nullptr;
                for (auto x : now->child)
                {
                    if (x->type == li)
                    {
                        ptr = x;
                    }
                }
                if (ptr != nullptr)
                {
                    make_paragraph(ptr);
                }
                flag = true;
            }
            now->child.push_back(new node(li));
            now = now->child.back();
            if (flag)
            {
                now->child.push_back(new node(p));
                now = now->child.back();
            }
            insert(now, string(tarl.real_loc));
        }

        //ordered list
        if (tarl.type == ol)
        {
            if (now->child.empty() || now->child.back()->type != ol)
            {
                now->child.push_back(new node(ol));
            }
            now = now->child.back();
            bool flag = false;
            if (new_p && !now->child.empty())
            {
                node *ptr = nullptr;
                for (auto x : now->child)
                {
                    if (x->type == li)
                        ptr = x;
                }
                if (ptr != nullptr)
                    make_paragraph(ptr);
                flag = true;
            }
            now->child
                .push_back(new node(li));
            now = now->child.back();
            if (flag)
            {
                now->child.push_back(new node(p));
                now = now->child.back();
            }
            insert(now, string(tarl.real_loc));
        }

        //quote
        if (tarl.type == quote)
        {
            if (now->child.empty() || now->child.back()->type != quote)
            {
                now->child.push_back(new node(quote));
            }
            now = now->child.back();
            if (new_p || now->child.empty())
                now->child.push_back(new node(p));
            insert(now->child.back(), string(tarl.real_loc));
        }

        new_p = false;
    }

    ifs.close();
    dfs(root);
    catalog += "<ul>";
    for (int i = 0; i < static_cast<int>(c_root->child.size()); ++i)
    {
        c_dfs(c_root->child[i], to_string(i + 1) + '.');
    }
    catalog += "</ul>";
}

#endif

#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stack>

#define SPACES       4

static inline void ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

static inline void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

static inline void backslash(std::string &s)
{
    size_t pos = 0;
    while ((pos = s.find_first_of('\\', pos)) != std::string::npos) {
        s.insert(pos, "\\");
        pos = pos + 2;
    }
}

#define USAGE      \
    "Usage: ./cmd <in cad file> <out json file>\n"

int main(int argc, char* argv[])
{
    if (argc < 3) {
        printf(USAGE);
        return -1;
    }
    
    std::ifstream ifs(argv[1], std::ifstream::in | std::ifstream::binary);
    if (! ifs.is_open()) {
        printf("open %s failed", argv[1]);
        return -1;
    }
    
    std::ofstream ofs(argv[2], std::ofstream::out | std::ofstream::binary);
    if (! ofs.is_open()) {
        printf("open %s failed", argv[2]);
        return -1;
    }
    
    std::ostringstream oss, toss;
    
    char buf[1024]{0};
    std::string line, prev_line;
    size_t pos;
    
    std::stack<char> stk;
    bool append_comma = false;

    oss << "{";
        
    while (! ifs.eof()) {
        memset(buf, 0, sizeof(buf));
        ifs.getline(buf, sizeof(buf));
        
        line.assign(buf, strlen(buf));
        ltrim(line);
        rtrim(line);
        backslash(line);
        
        toss.str("");
        
        if (line.empty()) {
            continue;
        }
        
        if (prev_line.empty()) {
            prev_line = line;
            continue;
        }
        
        // description{  0{
        if ((pos = prev_line.find('{')) != std::string::npos && prev_line.size() == pos + 1) {
            toss << std::string((stk.size() + 1) * SPACES, ' ');
            // 0{
            if (prev_line.find_first_not_of("0123456789") == pos) {
                toss << "{";
                stk.push('{');
            }
            // description{
            else {
                if (line.find_first_not_of("0123456789") == line.size() - 1) {
                    toss << "\""
                        << std::string(prev_line.begin(), prev_line.begin() + pos)
                        << "\" : [";
                    stk.push('[');
                } else {
                    toss << "\""
                        << std::string(prev_line.begin(), prev_line.begin() + pos)
                        << "\" : {";
                    stk.push('{');
                }
            }
            append_comma = false;
        } else if ((pos = prev_line.find('}')) != std::string::npos && pos == 0) {
            toss << std::string((stk.size() + 1) * SPACES, ' ');
            if (stk.top() == '{') {
                toss << '}';
            } else if (stk.top() == '[') {
                toss << ']';
            }
            size_t tpos;
            if ((tpos = line.find('{')) != std::string::npos && line.size() == tpos + 1) {
                append_comma = true;
            } else {
                append_comma = false;
            }           
            stk.pop();
        } else {
            toss << std::string((stk.size() + 1) * SPACES, ' ');
            if ((pos = prev_line.find('=')) != std::string::npos) {
                if (prev_line.find('"') == pos + 1) {
                    prev_line.append("\"")
                            .replace(pos, 1, 1, ':')
                            .insert(pos, 1, '"')
                            .insert(0, 1, '"');
                } else if (prev_line.find('"') != pos + 1 && prev_line.find('(') != std::string::npos) {
                    size_t tpos = prev_line.find_first_not_of("0123456789.", pos + sizeof('='));
                    prev_line.replace(prev_line.begin() + tpos, prev_line.end(), "")
                            .replace(pos, 1, 1, ':')
                            .insert(pos, 1, '"')
                            .insert(0, 1, '"');
                } else if (prev_line.find('"') != pos + 1 && prev_line.find('(') == std::string::npos) {
                    prev_line.replace(pos, 1, 1, ':')
                            .insert(pos, 1, '"')
                            .insert(0, 1, '"');
                    size_t tpos;
                    if ((tpos = prev_line.find("TRUE")) != std::string::npos) {
                        prev_line.replace(tpos, strlen("TRUE"), 1, '1');
                    } else if ((tpos = prev_line.find("FALSE")) != std::string::npos) {
                        prev_line.replace(tpos, strlen("FALSE"), 1, '0');
                    }
                } else {
                    printf("defined new parse method for this format yourself\n");
                }
            } else {
                printf("not key-value format\n");
                return -1;
            }
            
            
            toss << prev_line;
            if (line.find('}') != std::string::npos && line.size() == 1) {
                append_comma = false;
            } else {
                append_comma = true;
            }
        }
        
        
        if (append_comma) {
            toss << ",";
        }
        
        oss << std::endl << toss.str();
        
        prev_line = line;
    }
    // last line
    if (! stk.empty()) {
        oss << std::string((stk.size() + 1) * SPACES, ' ');
        if (stk.top() == '{') {
            oss << "}";
        } else if (stk.top() == '[') {
            oss << "]";
        }
        oss << std::endl;
        stk.pop();
    }
    
    oss << "}";

    ofs << oss.str();
    
    ifs.close();
    ofs.close();
    
    return 0;
}
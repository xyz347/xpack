#include <iostream>
#include <mysql/mysql.h>
#include <xpack/mysql.h>
#include <xpack/json.h>

using namespace std;

struct VID {
    int64_t Vid;
    int64_t vlen;
    string  sid;
    int64_t level;
    XPACK(A(Vid, "db:vid"), O(vlen, sid, level));
};

typedef char cfgstr[64];
struct config {
    cfgstr   host;
    uint32_t port;
    cfgstr   user;
    cfgstr   password;
    cfgstr   dbname;
    XPACK(M(host, port, user, password, dbname))
};

int main(int argc, char **argv) {
    MYSQL *mysql = NULL;

    if (mysql_library_init(argc, argv, NULL)) {
        cout<<"could not initialize MySQL client library"<<endl;
        return -1;
    }
 
    mysql = mysql_init(mysql);

    if (!mysql) {
        cout<<"Init faild, out of memory?"<<endl;
        return -1;
    }
        
    config cfg;
    xpack::json::decode_file("./config.json", cfg);

    if (!mysql_real_connect(mysql,        /* MYSQL structure to use */
                            cfg.host,     /* server hostname or IP address */ 
                            cfg.user,     /* mysql user */
                            cfg.password, /* password */
                            cfg.dbname,   /* default database to use, NULL for none */
                            cfg.port,     /* port number, 0 for default */
                            NULL,         /* socket file or named pipe name */
                            CLIENT_FOUND_ROWS /* connection flags */ )) {
        cout<<"Connect failed"<<endl;
    } else {              
        if (mysql_query(mysql, argv[1])) {
            cout<<"Query failed: "<<mysql_error(mysql)<<endl;
        } else {
            MYSQL_RES *result = mysql_store_result(mysql);

            if (!result) {
                cout<<"Couldn't get results set: "<<mysql_error(mysql)<<endl;
            } else {
                VID vid;
                xpack::mysql::decode(result, vid); // decode first row

                vector<VID> vids;
                xpack::mysql::decode(result, vids);// decode all rows

                vector<int64_t> ids;
                xpack::mysql::decode(result, "vid", ids); // decode vid column of all rows

                int64_t id;
                xpack::mysql::decode(result, "vid", id);  // decode vid column of first row

                cout<<xpack::json::encode(vid)<<endl;
                cout<<xpack::json::encode(vids)<<endl;
                cout<<xpack::json::encode(ids)<<endl;
                cout<<id<<endl;

                mysql_free_result(result);
            }
        }
    }
        
    mysql_close(mysql);

    mysql_library_end();
  
    return EXIT_SUCCESS;
}

#include <iostream>
#include <xpack/sqlite.h>
#include <xpack/json.h>

using namespace std;

struct VID {
    int64_t Vid;
    int64_t vlen;
    string  sid;
    int64_t level;
    XPACK(A(Vid, "db:vid"), O(vlen, sid, level));
};

int main(int argc, char **argv) {
    (void)argc;
    char *errMsg = NULL;
    sqlite *pDB = sqlite_open("./test.db", 0, &errMsg);
    if (NULL == pDB) {
        cout<<"open sqlite db fail:"<<errMsg<<endl;
        sqlite_freemem(errMsg);
        return -1;
    }

    int nCols = 0;
    int nRows = 0;
    char **azResult = NULL;

    int ret = sqlite_get_table(pDB, argv[1], &azResult, &nRows, &nCols, &errMsg);
    if (ret != SQLITE_OK) {
        cout<<"query sqlite db fail:"<<ret<<endl;
        return -1;
    }

    cout<<nRows<<','<<nCols<<endl;
    for (int i=0; i<nCols; ++i) {
        cout<<azResult[i]<<' ';
    }
    cout<<endl;
    int idx = nCols;
    for (int i=0; i<nRows; ++i) {
        for (int j=0; j<nCols; ++j, ++idx) {
            if (NULL == azResult[idx]) {
                cout<<"NULL"<<' ';
            } else {
                cout<<azResult[idx]<<' ';
            }
        }
        cout<<endl;
    }

    if (NULL != azResult) {
        sqlite_free_table(azResult);
    }
    if (NULL != errMsg) {
        sqlite_freemem(errMsg);
    }
    sqlite_close(pDB);

    return 0;
}

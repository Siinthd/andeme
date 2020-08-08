#include "sqliteutil.h"

namespace  {

    class SqliteGuard {
    public:
        SqliteGuard() {
            sqlite3_initialize();
        }
        ~SqliteGuard() {
            sqlite3_shutdown();
        }
    };
}

namespace andeme {

    SQLite3Common::SQLite3Common()
    {
        static SqliteGuard guard;
    }

    SQLite3Common::~SQLite3Common(){ }

    bool SQLite3Common::ExecuteQuery(const std::string& query,const Callback& callback)
    {
        return (SQLITE_OK == sqlite3_exec(db_, query.c_str(), sqlite_callback, (void *)(&callback), nullptr));
    }

    MessageStorage::MessageStorage(const std::string & dbname)
    {
        const std::string sql = "CREATE table IF NOT EXISTS MESSAGES ("
            "ID INTEGER PRIMARY KEY,"
            "Timestamp INTEGER NOT NULL,"
            "Author TEXT NOT NULL,"
            "Message TEXT NOT NULL,"
            "Signature TEXT NOT NULL ,UNIQUE(Timestamp,Message));";

        sqlite3_open_v2(dbname.data(), &db_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
        ExecuteQuery(sql.data(),nullptr);
    }
    MessageStorage::~MessageStorage()
    {
        sqlite3_close_v2(db_);
    }

    bool MessageStorage::AddMessage(const andeme::schema::Message & msg)
    {
        // message Message { string text = 1;}

        std::string Author = "Author";
        std::string sign = "sign";
        std::string sql =  "INSERT INTO MESSAGES ('Timestamp', 'Author', 'Message','Signature') \
                VALUES (datetime('now','localtime'),'"+ Author +"','"+ msg.text() +"','"+ sign +"');";
        return (ExecuteQuery(sql.data(),nullptr));
    }

    std::vector<andeme::schema::Message> MessageStorage::getAllMessages()
    {
        std::vector<andeme::schema::Message> messages;

        ExecuteQuery("SELECT Message FROM 'MESSAGES' ORDER BY ID ASC",[&messages](const Row& row)->bool{
            andeme::schema::Message msg;
            msg.set_text(row[0]);
            messages.push_back(std::move(msg));
            return true;
        });
        return messages;
    }

   static int sqlite_callback(void *NotUsed, int argc, char **argv, char **azColName)
    {
        andeme::Callback& callback = *static_cast<Callback*>(NotUsed);

            Row row;

            for (size_t i = 0; i < argc; ++i) {
                row.push_back(argv[i]);
            }
            if (callback(row))
                return SQLITE_OK;
    }
}

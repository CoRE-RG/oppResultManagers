#ifndef CSQLITEOUTPUTMANAGER_H_
#define CSQLITEOUTPUTMANAGER_H_

#include <sqlite3.h>
#include <unordered_map>

#include <omnetpp.h>

class cSQLiteOutputManager
{
    protected:
        static sqlite3* connection;
        static bool hasTransaction;
        static size_t users;

        size_t commitFreq;
        size_t insertCount;
        size_t runid;

        static std::unordered_map<std::string, size_t> moduleIDMap;
        static std::unordered_map<std::string, size_t> nameIDMap;

    public:
        /**
         * Constructor.
         */
        cSQLiteOutputManager();

        /**
         * Destructor.
         */
        virtual ~cSQLiteOutputManager();

        /**
         * Opens collecting. Called at the beginning of a simulation run.
         */
        virtual void startRun();

        /**
         * Closes collecting. Called at the end of a simulation run.
         */
        virtual void endRun();

        /**
         * Performs a database commit.
         */
        virtual void flush();

        /**
         * Returns nullptr, because this class doesn't use a file.
         */
        const char *getFileName() const
        {
            return nullptr;
        }
    protected:
        size_t getModuleID(std::string module);
        size_t getNameID(std::string name);
};

#endif /* CSQLITEOUTPUTMANAGER_H_ */

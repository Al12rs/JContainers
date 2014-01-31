#pragma once

namespace collections {
    // static unsigned long create(StaticFunctionTag *) {

    class shared_state {
        bshared_mutex _mutex;

        shared_state()
            : registry(_mutex)
            , aqueue(_mutex)
            , _databaseId(0)
        {
            setupForFirstTime();
        }

    public:
        collection_registry registry;
        autorelease_queue aqueue;
        HandleT _databaseId;

        static shared_state& instance() {
            static shared_state st;
            return st;
        }

        HandleT databaseId() {
            read_lock r(_mutex);
            return _databaseId;
        }

        void setDataBaseId(HandleT hdl) {
            setDataBase(registry.getObject(hdl));
        }

        object_base* database() {
            return registry.getObject(databaseId());
        }

        void setDataBase(object_base *db) {
            auto prev = registry.getObject(databaseId());
            if (prev == db) {
                return;
            }

            if (prev) {
                prev->release(); // may cause deadlock in removeObject
            }

            if (db) {
                db->retain();
            }

            write_lock g(_mutex);
            _databaseId = db ? db->id : 0;
        }

        void clearState();

        void loadAll(const vector<char> &data);

        void setupForFirstTime() {
            setDataBase(map::object());
        }

        vector<char> saveToArray();
    };

    inline autorelease_queue& autorelease_queue::instance() {
        return shared_state::instance().aqueue;
    }

    inline collection_registry& collection_registry::instance() {
        return shared_state::instance().registry;
    }
}


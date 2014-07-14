namespace collections {

    const char *kCommentObject = "creates new container object. returns container identifier (integral number).\n"
        "identifier is the thing you will have to pass to the most of container's functions as first argument";

#define ARGS(...)   #__VA_ARGS__

    class tes_object : public tes_binding::class_meta_mixin_t< tes_object > {
    public:

        typedef object_stack_ref& ref;

        REGISTER_TES_NAME("JValue");

        void additionalSetup() {
            metaInfo.comment = "Each container (JArray, JMap & JFormMap) inherits JValue functionality";
        }

        static object_base* retain(ref obj) {
            if (obj) {
                obj->tes_retain();
                return obj.get();
            }

            return nullptr;
        }
        REGISTERF2(retain, "*",
"Retains and returns the object.\n\
All containers that were created with object* or objectWith* methods are automatically destroyed after some amount of time (~10 seconds)\n\
To keep object alive you must retain it once and you have to __release__ it when you do not need it anymore (also to not pollute save file).\n\
An alternative to retain-release is store object in JDB container"
            );

        template<class T>
        static T* object() {
            return T::object(tes_context::instance());
        }

        static object_base* release(ref obj) {
            if (obj) {
                obj->tes_release();
            }

            return nullptr;
        }
        REGISTERF2(release, "*", "releases the object and returns zero, so you could release and nullify with one line of code: object = JVlaue.release(object)");

        static object_base* releaseAndRetain(ref previousObject, ref newObject) {
            if (previousObject != newObject) {
                if (previousObject) {
                    previousObject->tes_release();
                }

                if (newObject) {
                    newObject->tes_retain();
                }
            }

            return newObject.get();
        }
        REGISTERF2(releaseAndRetain, "previousObject newObject",
"just a union of retain-release calls. releases previousObject, retains and returns newObject.\n\
useful for those who use Papyrus properties instead of manual (and more error-prone) release-retain object lifetime management");

        static bool isArray(ref obj) {
            return obj->as<array>() != nullptr;
        }
        REGISTERF2(isArray, "*", "returns true if object is map, array or formmap container");

        static bool isMap(ref obj) {
            return obj->as<map>() != nullptr;
        }
        REGISTERF2(isMap, "*", NULL);

        static bool isFormMap(ref obj) {
            return obj->as<form_map>() != nullptr;
        }
        REGISTERF2(isFormMap, "*", NULL);

        static bool empty(ref obj) {
            return count(obj) == 0;
        }
        REGISTERF2(empty, "*", "returns true, if container is empty");

        static SInt32 count(ref obj) {
            return obj ? obj->s_count() : 0;
        }
        REGISTERF2(count, "*", "returns the number of items in container");

        static void clear(ref obj) {
            if (obj) {
                obj->s_clear();
            }
        }
        REGISTERF2(clear, "*", "removes all items from container");

        static object_base* readFromFile(const char *path) {
            auto obj = json_deserializer::object_from_file(tes_context::instance(), path);
            return  obj;
        }
        REGISTERF2(readFromFile, "filePath", "creates and returns new container object containing the contents of JSON file");

        static object_base* readFromDirectory(const char *dirPath, const char *extension = "")
        {
            using namespace boost;

            if (!dirPath || !filesystem::exists( dirPath )) {
                return nullptr;
            }

            if (!extension) {
                extension = "";
            }

            filesystem::directory_iterator end_itr;
            filesystem::path root(dirPath);

            map *files = map::object(tes_context::instance()); 

            for ( filesystem::directory_iterator itr( root ); itr != end_itr; ++itr ) {

                if ( filesystem::is_regular_file( *itr ) &&
                     (!*extension || itr->path().extension().generic_string().compare(extension) == 0) ) {
                    auto asniString = itr->path().generic_string();
                    auto jsonObject = tes_object::readFromFile(asniString.c_str());

                    if (jsonObject) {
                        files->setValueForKey(itr->path().filename().generic_string(), Item(jsonObject));
                    }  
                }
            }

            return files;
        }
        REGISTERF2(readFromDirectory, "directoryPath extension=\"\"",
            "parses JSON files in directory (non recursive) and returns JMap containing {filename, container-object} pairs.\n"
            "note: by default it does not filters files by extension and will try to parse everything");

        static object_base* objectFromPrototype(const char *prototype) {
            auto obj = json_deserializer::object_from_json_data( tes_context::instance(), prototype);
            return obj;
        }
        REGISTERF2(objectFromPrototype, "prototype", "creates new container object using given JSON string-prototype");

        static void writeToFile(object_base* obj, const char * path) {
            if (!path || !obj) {
                return;
            }

            auto json = json_serializer::create_json_value(*obj);
            if (json) {
                json_dump_file(json.get(), path, JSON_INDENT(2));
            }
        }

        static void _writeToFile(ref obj, const char * path) {
            writeToFile(obj.get(), path);
        }
        REGISTERF(_writeToFile, "writeToFile", "* filePath", "writes object into JSON file");

        static bool hasPath(object_base* obj, const char *path) {
            if (!obj || !path)
                return false;

            bool succeed = false;
            path_resolving::resolvePath(obj, path, [&](Item* itmPtr) {
                succeed = (itmPtr != nullptr);
            });

            return succeed;
        }

        static bool _hasPath(ref obj, const char *path) {
            return hasPath(obj.get(), path);
        }
        REGISTERF(_hasPath, "hasPath", "* path",
"returns true, if container capable resolve given path.\n\
for ex. JValue.hasPath(container, \".player.health\") will check if given container has 'player' which has 'health' information"
                                      );

        template<class T>
        static T resolveGetter(object_base *obj, const char* path) {
            if (!obj || !path)
                return 0;

            T val((T)0);
            path_resolving::resolvePath(obj, path, [&](Item* itmPtr) {
                if (itmPtr) {
                    val = itmPtr->readAs<T>();
                }
            });

            return val;
        }
        template<class T>
        static T _resolveGetter(ref obj, const char* path) {
            return resolveGetter<T>(obj.get(), path);
        }
        REGISTERF(_resolveGetter<Float32>, "solveFlt", "* path", "attempts to get value at given path.\nJValue.solveInt(container, \".player.mood\") will return player's mood");
        REGISTERF(_resolveGetter<SInt32>, "solveInt", "* path", NULL);
        REGISTERF(_resolveGetter<const char*>, "solveStr", "* path", NULL);
        REGISTERF(_resolveGetter<object_base*>, "solveObj", "* path", NULL);
        REGISTERF(_resolveGetter<TESForm*>, "solveForm", "* path", NULL);

        template<class T>
        static bool solveSetter(object_base* obj, const char* path, T value) {
            if (!obj || !path)
                return false;

            bool succeed = false;
            path_resolving::resolvePath(obj, path, [&](Item* itmPtr) {
                if (itmPtr) {
                    *itmPtr = Item((T)value);
                    succeed = true;
                }
            });

            return succeed;
        }

        template<class T>
        static bool _solveSetter(ref obj, const char* path, T value) {
            return solveSetter<T>(obj.get(), path, value);
        }
        REGISTERF(_solveSetter<Float32>, "solveFltSetter", "* path value", "attempts to set value.\nJValue.solveIntSetter(container, \".player.mood\", 12) will set player's mood to 12");
        REGISTERF(_solveSetter<SInt32>, "solveIntSetter", "* path value", NULL);
        REGISTERF(_solveSetter<const char*>, "solveStrSetter", "* path value", NULL);
        REGISTERF(_solveSetter<ref>, "solveObjSetter", "* path value", NULL);
        REGISTERF(_solveSetter<TESForm*>, "solveFormSetter", "* path value", NULL);

    };

    TES_META_INFO(tes_object);

}

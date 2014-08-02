#include <cstdint>
#include "jcontainers_constants.h"

namespace jc {

    enum message {
        message_none,
        message_root_interface,
    };

    struct root_interface {

        enum {
            version = 1,
        };

        uint32_t current_version;

        const void * (*_query_interface)(uint32_t id);

        template<class Intrfc>
        const Intrfc * query_interface() const {
            auto intr = (const Intrfc *)_query_interface(Intrfc::type_id);
            return intr->current_version == Intrfc::version ? intr : nullptr;
        }

        static const root_interface * from_void(void * root) {
            return ((root_interface *)root)->current_version == version ? (root_interface *)root : nullptr;
        }
    };

    struct reflection_interface {

        enum {
            type_id = 1,
            version = collections::kJAPIVersion,
        };

        uint32_t current_version;

        void * (*tes_function_of_class)(const char *function_name, const char *class_name);
    };
}

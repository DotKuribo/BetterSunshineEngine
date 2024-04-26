#include <Enemy/Conductor.hxx>
#include <MapObj/MapObjBase.hxx>
#include <System/Application.hxx>
#include <System/MarNameRefGen.hxx>
#include <System/PerformList.hxx>

#include "module.hxx"
#include <JDrama/JDRViewObjPtrListT.hxx>

#if 0 && SMS_DEBUG

enum class ETask {
    NONE,
    GET_NAMEREF_PTR,
    CREATE_NAMEREF,
    DELETE_NAMEREF,
    PLAY_CAMERA_DEMO,
    SET_NAMEREF_PARAMETER
};

static ETask *s_requested_task    = (ETask *)0x800002E0;
static void **s_request_buffer_p  = (void **)0x800002E4;
static void **s_response_buffer_p = (void **)0x800002E8;

static char s_request_buffer[0x10000] = {};
static char s_response_buffer[0x100]  = {};

BETTER_SMS_FOR_CALLBACK void initializeTaskBuffers() {
    OSReport("Initializing toolbox buffers\n");
    *s_request_buffer_p  = &s_request_buffer;
    *s_response_buffer_p = &s_response_buffer;
}

BETTER_SMS_FOR_CALLBACK void processCurrentTask(TApplication *app) {
    bool is_directing_scene = app->mContext == TApplication::CONTEXT_DIRECT_STAGE;
    switch (*s_requested_task) {
    default:
    case ETask::NONE:
        OSReport("Task is none\n");
        return;
    case ETask::GET_NAMEREF_PTR: {
        const char *actor_name = *(const char **)(s_request_buffer_p);
        **reinterpret_cast<JDrama::TNameRef ***>(s_response_buffer_p) =
            app->mDirector->search(actor_name);
        *s_requested_task = ETask::NONE;
        return;
    }
    case ETask::CREATE_NAMEREF: {
        if (!is_directing_scene) {
            *s_requested_task = ETask::NONE;
            return;
        }

        // Corrupted type?
        const char *parent_type = *(const char **)(s_request_buffer_p);
        if (strlen(parent_type) >= 0x80) {
            OSReport("(CREATE_NAMEREF) Toolbox parent type is corrupt!\n");
            *s_requested_task = ETask::NONE;
            return;
        }

        if (strcmp(parent_type, "IdxGroup") != 0) {
            OSReport("(CREATE_NAMEREF) Toolbox parent type is unsupported!\n");
            *s_requested_task = ETask::NONE;
            return;
        }

        // Corrupted name?
        const char *parent_name = *(const char **)(s_request_buffer_p) + 0x80;
        if (strlen(parent_name) >= 0x180) {
            OSReport("(CREATE_NAMEREF) Toolbox parent name is corrupt!\n");
            *s_requested_task = ETask::NONE;
            return;
        }

        JDrama::TNameRef *parent_ref =
            TMarNameRefGen::getInstance()->getRootNameRef()->search(parent_name);
        if (!parent_ref) {
            OSReport("(CREATE_NAMEREF) Toolbox parent reference not found!\n");
            *s_requested_task = ETask::NONE;
            return;
        }

        size_t data_size = *(size_t *)(*(const char **)(s_request_buffer_p) + 0x200);
        JSUMemoryInputStream stream((void *)(*(const char **)(s_request_buffer_p) + 0x200),
                                    data_size);
        JSUMemoryInputStream other(nullptr, 0);

        JDrama::TNameRef *generated = JDrama::TNameRef::genObject(stream, other);
        if (!generated) {
            OSReport("(CREATE_NAMEREF) Toolbox nameref data was invalid!\n");
            *s_requested_task = ETask::NONE;
            return;
        }

        /*if (name == "MapStaticObj") {
            reinterpret_cast<TMapStaticObj *>(generated)->init();
        }*/

        if (strcmp(parent_type, "IdxGroup") == 0) {
            auto *parent_view_obj_ref =
                reinterpret_cast<JDrama::TViewObjPtrListT<JDrama::TViewObj> *>(parent_ref);
            parent_view_obj_ref->mViewObjList.push_back(generated);
        }

        generated->load(other);
        generated->loadAfter();

        *((JDrama::TNameRef **)(s_response_buffer_p)) = generated;
        *s_requested_task                             = ETask::NONE;
        return;
    }
    case ETask::DELETE_NAMEREF: {
        if (!is_directing_scene) {
            *s_requested_task = ETask::NONE;
            return;
        }

        // Corrupted type?
        const char *parent_type = *(const char **)(s_request_buffer_p);
        if (strlen(parent_type) >= 0x80) {
            OSReport("(DELETE_NAMEREF) Toolbox parent type is corrupt!\n");
            *s_requested_task = ETask::NONE;
            return;
        }

        if (strcmp(parent_type, "IdxGroup") != 0) {
            OSReport("(DELETE_NAMEREF) Toolbox parent type is unsupported!\n");
            *s_requested_task = ETask::NONE;
            return;
        }

        // Corrupted name?
        const char *parent_name = *(const char **)(s_request_buffer_p) + 0x80;
        if (strlen(parent_name) >= 0x180) {
            OSReport("(DELETE_NAMEREF) Toolbox parent name is corrupt!\n");
            *s_requested_task = ETask::NONE;
            return;
        }

        // Corrupted name?
        const char *this_name = *(const char **)(s_request_buffer_p) + 0x200;
        if (strlen(this_name) >= 0x180) {
            OSReport("(DELETE_NAMEREF) Toolbox this name is corrupt!\n");
            *s_requested_task = ETask::NONE;
            return;
        }

        JDrama::TNameRef *parent_ref =
            TMarNameRefGen::getInstance()->getRootNameRef()->search(parent_name);
        if (!parent_ref) {
            OSReport("(DELETE_NAMEREF) Toolbox parent reference not found!\n");
            *s_requested_task = ETask::NONE;
            return;
        }

        if (strcmp(parent_type, "IdxGroup") == 0) {
            JDrama::TViewObj *view_obj;

            auto *parent_view_obj_ref =
                reinterpret_cast<JDrama::TViewObjPtrListT<JDrama::TViewObj> *>(parent_ref);

            for (auto it = parent_view_obj_ref->mViewObjList.begin();
                 it != parent_view_obj_ref->mViewObjList.end(); ++it) {
                view_obj = (JDrama::TViewObj *)(*it);
                if (strcmp(view_obj->mKeyName, this_name) == 0) {
                    parent_view_obj_ref->mViewObjList.erase(it);
                    break;
                }
            }

            for (auto it = gpConductor->_10.begin(); it != gpConductor->_10.end(); ++it) {
                auto *obj_manager_ref = (TObjManager *)(*it);
                for (size_t i = 0; i < obj_manager_ref->mObjCount; ++i) {
                    if (obj_manager_ref->mObjAry[i] == view_obj) {
                        memmove(&obj_manager_ref->mObjAry[i], &obj_manager_ref->mObjAry[i + 1],
                                (obj_manager_ref->mObjCount - i) * 4);
                        obj_manager_ref->mObjCount -= 1;
                        break;
                    }
                }
            }

            /*for (auto it = gpConductor->_20.begin(); it != gpConductor->_20.end(); ++it) {
                auto *obj_manager_ref = (TObjManager *)(*it);
                for (size_t i = 0; i < obj_manager_ref->mObjCount; ++i) {
                    if (obj_manager_ref->mObjAry[i] == view_obj) {
                        memmove(&obj_manager_ref->mObjAry[i], &obj_manager_ref->mObjAry[i + 1],
                                (obj_manager_ref->mObjCount - i) * 4);
                        obj_manager_ref->mObjCount -= 1;
                        break;
                    }
                }
            }

            for (auto it = gpConductor->_30.begin(); it != gpConductor->_30.end(); ++it) {
                auto *obj_manager_ref = (TObjManager *)(*it);
                for (size_t i = 0; i < obj_manager_ref->mObjCount; ++i) {
                    if (obj_manager_ref->mObjAry[i] == view_obj) {
                        memmove(&obj_manager_ref->mObjAry[i], &obj_manager_ref->mObjAry[i + 1],
                                (obj_manager_ref->mObjCount - i) * 4);
                        obj_manager_ref->mObjCount -= 1;
                        break;
                    }
                }
            }

            for (auto it = gpConductor->_40.begin(); it != gpConductor->_40.end(); ++it) {
                auto *obj_manager_ref = (TObjManager *)(*it);
                for (size_t i = 0; i < obj_manager_ref->mObjCount; ++i) {
                    if (obj_manager_ref->mObjAry[i] == view_obj) {
                        memmove(&obj_manager_ref->mObjAry[i], &obj_manager_ref->mObjAry[i + 1],
                                (obj_manager_ref->mObjCount - i) * 4);
                        obj_manager_ref->mObjCount -= 1;
                        break;
                    }
                }
            }

            for (auto it = gpConductor->_50.begin(); it != gpConductor->_50.end(); ++it) {
                auto *obj_manager_ref = (TObjManager *)(*it);
                for (size_t i = 0; i < obj_manager_ref->mObjCount; ++i) {
                    if (obj_manager_ref->mObjAry[i] == view_obj) {
                        memmove(&obj_manager_ref->mObjAry[i], &obj_manager_ref->mObjAry[i + 1],
                                (obj_manager_ref->mObjCount - i) * 4);
                        obj_manager_ref->mObjCount -= 1;
                        break;
                    }
                }
            }

            for (auto it = gpConductor->_60.begin(); it != gpConductor->_60.end(); ++it) {
                auto *obj_manager_ref = (TObjManager *)(*it);
                for (size_t i = 0; i < obj_manager_ref->mObjCount; ++i) {
                    if (obj_manager_ref->mObjAry[i] == view_obj) {
                        memmove(&obj_manager_ref->mObjAry[i], &obj_manager_ref->mObjAry[i + 1],
                                (obj_manager_ref->mObjCount - i) * 4);
                        obj_manager_ref->mObjCount -= 1;
                        break;
                    }
                }
            }

            for (auto it = gpConductor->_70.begin(); it != gpConductor->_70.end(); ++it) {
                auto *obj_manager_ref = (TObjManager *)(*it);
                for (size_t i = 0; i < obj_manager_ref->mObjCount; ++i) {
                    if (obj_manager_ref->mObjAry[i] == view_obj) {
                        memmove(&obj_manager_ref->mObjAry[i], &obj_manager_ref->mObjAry[i + 1],
                                (obj_manager_ref->mObjCount - i) * 4);
                        obj_manager_ref->mObjCount -= 1;
                        break;
                    }
                }
            }*/

            if (view_obj)
                delete view_obj;
        }

        *s_requested_task = ETask::NONE;
    }
    case ETask::PLAY_CAMERA_DEMO: {
        if (!is_directing_scene) {
            *s_requested_task = ETask::NONE;
            return;
        }

        // Corrupted type?
        const char *demo_name = *(const char **)(s_request_buffer_p) + 0x4;
        if (strlen(demo_name) >= 0x7C) {
            OSReport("(CAMERA_DEMO) Toolbox demo name is corrupt!\n");
            *s_requested_task = ETask::NONE;
            return;
        }

        JDrama::TFlagT<u16> flags(0);
        ((TMarDirector *)app->mDirector)
            ->fireStartDemoCamera(demo_name, nullptr, -1, 0.0f, true, nullptr, 0, nullptr, flags);

        *s_requested_task = ETask::NONE;
        return;
    }
    case ETask::SET_NAMEREF_PARAMETER: {
        // ... Will implement later
        *s_requested_task = ETask::NONE;
        return;
    }
    }
}

#else

BETTER_SMS_FOR_CALLBACK void initializeTaskBuffers() {}

BETTER_SMS_FOR_CALLBACK void processCurrentTask(TApplication *app) {}

#endif
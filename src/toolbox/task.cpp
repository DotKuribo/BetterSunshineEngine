#include <Enemy/Conductor.hxx>
#include <MapObj/MapObjBase.hxx>
#include <System/Application.hxx>
#include <System/MarNameRefGen.hxx>
#include <System/PerformList.hxx>

#include "module.hxx"
#include <JDrama/JDRViewObjPtrListT.hxx>

#ifdef BETTER_SMS_INCLUDE_DEBUGGER_STUB

enum class ETask {
    NONE                  = 0,
    GET_NAMEREF_PTR       = 1,
    CREATE_NAMEREF        = 2,
    DELETE_NAMEREF        = 3,
    SET_NAMEREF_PARAMETER = 4,
    PLAY_CAMERA_DEMO      = 5,
    UPDATE_SCENE_ARCHIVE  = 6,
    CHANGE_SCENE          = 7,
};

#define STR_EQUAL(a, b) ((bool)(strcmp((a), (b)) == 0))

// Interface functions
static void setErrorResponse(const char *msg);
static void clearResponse();
static void *getResponseBuffer();
static u32 getResponseBufferSize();
static ETask getRequestedTask();
static void clearRequestedTask();
static void *getRequestBuffer();
static u32 getRequestBufferSize();

namespace _impl {
    static ETask *s_requested_task    = (ETask *)0x800002E0;
    static void **s_request_buffer_p  = (void **)0x800002E4;
    static void **s_response_buffer_p = (void **)0x800002E8;

    static char s_request_buffer[0x10000] = {};
    static char s_response_buffer[0x100]  = {};
}  // namespace _impl

static void setErrorResponse(const char *msg) {
    _impl::s_response_buffer[0] = -1;
    strncpy((char *)getResponseBuffer(), msg, getResponseBufferSize());
}

static void setSuccessResponse() { _impl::s_response_buffer[0] = 1; }

static void clearResponse() { _impl::s_response_buffer[0] = 0; }

static void *getResponseBuffer() { return _impl::s_response_buffer + 4; }
static u32 getResponseBufferSize() { return sizeof(_impl::s_response_buffer) - 4; }

static ETask getRequestedTask() { return *_impl::s_requested_task; }
static void clearRequestedTask() { *_impl::s_requested_task = ETask::NONE; }

static void *getRequestBuffer() { return _impl::s_request_buffer; }
static u32 getRequestBufferSize() { return sizeof(_impl::s_request_buffer); }

static bool s_stage_transition_from_application = false;

BETTER_SMS_FOR_CALLBACK void initializeTaskBuffers() {
    OSReport("Initializing Junior's Toolbox buffers\n");
    *_impl::s_request_buffer_p  = &_impl::s_request_buffer;
    *_impl::s_response_buffer_p = &_impl::s_response_buffer;
}

BETTER_SMS_FOR_CALLBACK void processCurrentTask(TApplication *app) {
    bool is_directing_scene = app->mContext == TApplication::CONTEXT_DIRECT_STAGE;

    void *request_buffer          = getRequestBuffer();
    const u32 request_buffer_size = getRequestBufferSize();

    void *response_buffer          = getResponseBuffer();
    const u32 response_buffer_size = getResponseBufferSize();

    clearResponse();

    while (getRequestedTask() != ETask::NONE) {
        switch (getRequestedTask()) {
        default:
        case ETask::NONE:
            OSReport("[TOOLBOX] Task is none\n");
            return;
        case ETask::GET_NAMEREF_PTR: {
            const char *actor_name = (const char *)request_buffer;
            *reinterpret_cast<JDrama::TNameRef **>(response_buffer) =
                app->mDirector->search(actor_name);
            clearRequestedTask();
            setSuccessResponse();
            return;
        }
        case ETask::CREATE_NAMEREF: {
            if (!is_directing_scene) {
                setErrorResponse("(CREATE_NAMEREF) Scene is not ready yet!\n");
                clearRequestedTask();
                return;
            }

            // Corrupted type?
            const char *parent_type = (const char *)(request_buffer);
            if (strlen(parent_type) >= 0x80) {
                setErrorResponse("(CREATE_NAMEREF) Toolbox parent type is corrupt!\n");
                clearRequestedTask();
                return;
            }

            if (!STR_EQUAL(parent_type, "IdxGroup")) {
                setErrorResponse("(CREATE_NAMEREF) Toolbox parent type is unsupported!\n");
                clearRequestedTask();
                return;
            }

            // Corrupted name?
            const char *parent_name = (const char *)(request_buffer) + 0x80;
            if (strlen(parent_name) >= 0x180) {
                setErrorResponse("(CREATE_NAMEREF) Toolbox parent name is corrupt!\n");
                clearRequestedTask();
                return;
            }

            JDrama::TNameRef *parent_ref =
                TMarNameRefGen::getInstance()->getRootNameRef()->search(parent_name);
            if (!parent_ref) {
                setErrorResponse("(CREATE_NAMEREF) Toolbox parent reference not found!\n");
                clearRequestedTask();
                return;
            }

            size_t data_size = *(size_t *)((const char *)(request_buffer) + 0x200);
            JSUMemoryInputStream stream((void *)((const char *)(request_buffer) + 0x200),
                                        data_size);
            JSUMemoryInputStream other(nullptr, 0);

            JDrama::TNameRef *generated = JDrama::TNameRef::genObject(stream, other);
            if (!generated) {
                setErrorResponse("(CREATE_NAMEREF) Toolbox nameref data was invalid!\n");
                clearRequestedTask();
                return;
            }

            /*if (name == "MapStaticObj") {
                reinterpret_cast<TMapStaticObj *>(generated)->init();
            }*/

            if (STR_EQUAL(parent_type, "IdxGroup")) {
                auto *parent_view_obj_ref =
                    reinterpret_cast<JDrama::TViewObjPtrListT<JDrama::TViewObj> *>(parent_ref);
                parent_view_obj_ref->mViewObjList.push_back(generated);
            }

            generated->load(other);
            generated->loadAfter();

            *((JDrama::TNameRef **)response_buffer) = generated;
            clearRequestedTask();
            setSuccessResponse();
            return;
        }
        case ETask::DELETE_NAMEREF: {
            if (!is_directing_scene) {
                setErrorResponse("(DELETE_NAMEREF) Scene is not ready yet!\n");
                clearRequestedTask();
                return;
            }

            // Corrupted type?
            const char *parent_type = (const char *)(request_buffer);
            if (strlen(parent_type) >= 0x80) {
                setErrorResponse("(DELETE_NAMEREF) Toolbox parent type is corrupt!\n");
                clearRequestedTask();
                return;
            }

            if (!STR_EQUAL(parent_type, "IdxGroup")) {
                setErrorResponse("(DELETE_NAMEREF) Toolbox parent type is unsupported!\n");
                clearRequestedTask();
                return;
            }

            // Corrupted name?
            const char *parent_name = (const char *)(request_buffer) + 0x80;
            if (strlen(parent_name) >= 0x180) {
                setErrorResponse("(DELETE_NAMEREF) Toolbox parent NameRef is corrupt!\n");
                clearRequestedTask();
                return;
            }

            // Corrupted name?
            const char *this_name = (const char *)(request_buffer) + 0x200;
            if (strlen(this_name) >= 0x180) {
                setErrorResponse("(DELETE_NAMEREF) Toolbox target NameRef is corrupt!\n");
                clearRequestedTask();
                return;
            }

            JDrama::TNameRef *parent_ref =
                TMarNameRefGen::getInstance()->getRootNameRef()->search(parent_name);
            if (!parent_ref) {
                setErrorResponse("(DELETE_NAMEREF) Toolbox parent reference not found!\n");
                clearRequestedTask();
                return;
            }

            if (STR_EQUAL(parent_type, "IdxGroup")) {
                JDrama::TViewObj *view_obj;

                auto *parent_view_obj_ref =
                    reinterpret_cast<JDrama::TViewObjPtrListT<JDrama::TViewObj> *>(parent_ref);

                for (auto it = parent_view_obj_ref->mViewObjList.begin();
                     it != parent_view_obj_ref->mViewObjList.end(); ++it) {
                    view_obj = (JDrama::TViewObj *)(*it);
                    if (STR_EQUAL(view_obj->mKeyName, this_name)) {
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

            clearRequestedTask();
            setSuccessResponse();
            return;
        }
        case ETask::SET_NAMEREF_PARAMETER: {
            // ... Will implement later
            //setErrorResponse("(SET_NAMEREF_PARAMETER) Not implemented yet!\n");
            // Corrupted type?
            const char *obj_type = (const char *)(request_buffer);
            if (strlen(obj_type) >= 0x80) {
                setErrorResponse("(CAMERA_DEMO) Toolbox demo name is corrupt!\n");
                clearRequestedTask();
                return;
            }

            const char *obj_cur_name = (const char *)(request_buffer) + 0x80;
            if (strlen(obj_cur_name) >= 0x180) {
                setErrorResponse("(CAMERA_DEMO) Toolbox demo name is corrupt!\n");
                clearRequestedTask();
                return;
            }

            const char *obj_new_name = (const char *)(request_buffer) + 0x200;
            if (strlen(obj_new_name) >= 0x180) {
                setErrorResponse("(CAMERA_DEMO) Toolbox demo name is corrupt!\n");
                clearRequestedTask();
                return;
            }

            u16 obj_new_keycode = *(const u16 *)((const char *)(request_buffer) + 0x380);

            JDrama::TNameRef *object = app->mDirector->search(obj_cur_name);
            if (!object) {
                setErrorResponse("(SET_NAMEREF_PARAMETER) Could not find the target object!");
                clearRequestedTask();
                return;
            }

            // Yes this is a memory leak, but stages are allocated
            // using a solid heap so free operations fail anyway.
            // ---
            // The expectation by both the game and this tooling is
            // to periodically restart to refresh changes anyway.
            char *new_key_name = new char[0x180];
            strncpy(new_key_name, obj_new_name, 0x180);

            u16 sanity_code = JDrama::TNameRef::calcKeyCode(new_key_name);
            if (sanity_code != obj_new_keycode) {
                OSReport("[Junior's Toolbox] Warning: external tool has provided a malformed "
                         "keycode! (in: %d, exp: %d)\n",
                         obj_new_keycode, sanity_code);
            }
            object->mKeyCode = obj_new_keycode;
            object->mKeyName = new_key_name;

            setSuccessResponse();
            clearRequestedTask();
            return;
        }
        case ETask::PLAY_CAMERA_DEMO: {
            if (!is_directing_scene) {
                setErrorResponse("(CAMERA_DEMO) Scene is not ready yet!\n");
                clearRequestedTask();
                return;
            }

            // Corrupted type?
            const char *demo_name = (const char *)(request_buffer);
            if (strlen(demo_name) >= 0x80) {
                setErrorResponse("(CAMERA_DEMO) Toolbox demo name is corrupt!\n");
                clearRequestedTask();
                return;
            }

            JDrama::TFlagT<u16> flags(0);
            ((TMarDirector *)app->mDirector)
                ->fireStartDemoCamera(demo_name, nullptr, -1, 0.0f, true, nullptr, 0, nullptr,
                                      flags);

            clearRequestedTask();
            setSuccessResponse();
            return;
        }
        case ETask::UPDATE_SCENE_ARCHIVE: {
            // ... Will implement later
            setErrorResponse("(UPDATE_SCENE_ARCHIVE) Not implemented yet!\n");
            clearRequestedTask();
            return;
        }
        case ETask::CHANGE_SCENE: {
            u8 scene        = *(u8 *)request_buffer;
            u8 scenario     = *(u8 *)((const char *)request_buffer + 1);
            if (!is_directing_scene) {
                /*setErrorResponse("(CHANGE_SCENE) Scene is not ready yet!\n");
                clearRequestedTask();*/
                app->mNextScene.mAreaID    = scene;
                app->mNextScene.mEpisodeID = scenario;
                s_stage_transition_from_application = true;
                return;
            }
            if (!s_stage_transition_from_application) {
                u32 stageID = ((scene + 1) << 8) | scenario;
                ((TMarDirector *)app->mDirector)->setNextStage(stageID, 0);
            }
            s_stage_transition_from_application = false;
            clearRequestedTask();
            setSuccessResponse();
            return;
        }
        }
    }
}

#else

BETTER_SMS_FOR_CALLBACK void initializeTaskBuffers() {}

BETTER_SMS_FOR_CALLBACK void processCurrentTask(TApplication *app) {}

#endif
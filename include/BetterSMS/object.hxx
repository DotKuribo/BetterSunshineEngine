#pragma once

#include <JSystem/JDrama/JDRNameRef.hxx>
#include <SMS/MapObj/MapObjInit.hxx>

namespace BetterSMS {
    namespace Objects {
        typedef JDrama::TNameRef *(*NameRefInitializer)();
        typedef void (*ObjectInteractor)(THitActor *object, TMario *player);

        size_t getRegisteredObjectCount();
        size_t getRegisteredCustomObjectCount();
        size_t getRemainingCapacity();

        bool isObjectRegistered(const char *name);
        // Map objects (coins, blocks, etc)
        bool registerObjectAsMapObj(const char *name, ObjData *data, NameRefInitializer initFn);
        // Enemys (Strollin' Stus, Electrokoopas, etc)
        bool registerObjectAsEnemy(const char *name, ObjData *data, NameRefInitializer initFn);
        // Misc (Managers, tables, etc)
        bool registerObjectAsMisc(const char *name, NameRefInitializer initFn);
        // Player touching
        bool registerObjectCollideInteractor(u32 objectID, ObjectInteractor colHandler);
        // Player holding
        bool registerObjectGrabInteractor(u32 objectID, ObjectInteractor grabHandler);

        bool deregisterObject(const char *name);
    }  // namespace Objects
}  // namespace BetterSMS::Objects
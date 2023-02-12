#pragma once

#include <JSystem/JDrama/JDRNameRef.hxx>
#include <SMS/MapObj/MapObjInit.hxx>

namespace BetterSMS {
    namespace Objects {
        typedef JDrama::TNameRef *(*NameRefInitializer)();
        typedef void (*ObjectInteractor)(THitActor *object, TMario *player);

        // Get how many more objects can be registered
        size_t getRemainingCapacity();

        // Register a map object initializer (coins, blocks, etc)
        bool registerObjectAsMapObj(const char *name, ObjData *data, NameRefInitializer initFn);
        // Register an enemy initializer (Strollin' Stus, Electrokoopas, etc)
        bool registerObjectAsEnemy(const char *name, ObjData *data, NameRefInitializer initFn);
        // Register a miscellaneous object initializer (Managers, tables, etc)
        bool registerObjectAsMisc(const char *name, NameRefInitializer initFn);
        // Register a function to be called when an object collides with the player
        bool registerObjectCollideInteractor(u32 objectID, ObjectInteractor colHandler);
        // Register a function to be called when an object is being grabbed by the player
        bool registerObjectGrabInteractor(u32 objectID, ObjectInteractor grabHandler);

        bool deregisterObject(const char *name);
    }  // namespace Objects
}  // namespace BetterSMS::Objects
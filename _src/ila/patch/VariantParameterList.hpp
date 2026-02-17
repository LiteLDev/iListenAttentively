#pragma once

class Actor;
class Mob;
class Player;
class Player;
class BlockPos;

namespace ila::inline patch
{

struct VariantParameterList
{
    Actor*    mSelf    = nullptr;
    Actor*    mOther   = nullptr;
    Player*   mPlayer  = nullptr;
    Actor*    mTarget  = nullptr;
    Actor*    mParent  = nullptr;
    Mob*      mBaby    = nullptr;
    BlockPos* mBlock   = nullptr;
    Actor*    mDamager = nullptr;
    Actor*    mHolder  = nullptr;
};

} // namespace ila::inline patch
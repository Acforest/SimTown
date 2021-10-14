#pragma once
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
// 第一人称相机
class FirstPersonCamera
{
private:
    XMFLOAT4 mPosition;     // 位置
    XMFLOAT4 mTarget;       // 看的位置
    XMFLOAT4 mUpAxis;       // 上方向（局部坐标系的+Y轴）
    XMFLOAT4 mRightAxis;    // 右方向（局部坐标系的+X轴）
    int up_down;            // 跳跃状态（0表示在地面，1表示在上升，-1表示在下降）
    int up_down_lock;       // 跳跃状态锁
public:
    FirstPersonCamera(XMVECTOR& mPosition, XMVECTOR& mTarget, XMVECTOR& mUpAxis, XMVECTOR& mRightAxis)
    {
        XMStoreFloat4(&this->mPosition, mPosition);
        XMStoreFloat4(&this->mTarget, mTarget);
        XMStoreFloat4(&this->mUpAxis, mUpAxis);
        XMStoreFloat4(&this->mRightAxis, mRightAxis);
        up_down = 0;
        up_down_lock = 0;
    }
    XMFLOAT4 GetPosition()
    {
        return mPosition;
    }
    XMFLOAT4 GetTarget()
    {
        return mTarget;
    }
    XMFLOAT4 GetUpAxis()
    {
        return mUpAxis;
    }
    XMFLOAT4 GetRightAxis()
    {
        return mRightAxis;
    }
    // 左右移动
    void MoveLeftRight(float d)
    {
        float normalize_Length = mRightAxis.x * mRightAxis.x + mRightAxis.z * mRightAxis.z;
        float normalize_X = mRightAxis.x / normalize_Length;
        float normalize_Z = mRightAxis.z / normalize_Length;
        mPosition.x += normalize_X * d;
        mPosition.z += normalize_Z * d;
    }
    // 前后移动
    void MoveForwardBack(float d)
    {
        float normalize_Length = mTarget.x * mTarget.x + mTarget.z * mTarget.z;
        float normalize_X = mTarget.x / normalize_Length;
        float normalize_Z = mTarget.z / normalize_Length;
        mPosition.x += normalize_X * d;
        mPosition.z += normalize_Z * d;
    }
    // 抬头低头（绕局部坐标系的X轴旋转）
    void Pitch(float rad)
    {
        // 绕局部坐标系的X轴旋转
        XMFLOAT4 saveTarget = mTarget;
        XMVECTOR newTarget = XMVector4Transform(XMLoadFloat4(&mTarget), XMMatrixRotationAxis(XMLoadFloat4(&mRightAxis), rad));
        XMStoreFloat4(&mTarget, newTarget);
        // 防止万向节死锁
        //if (mTarget.y < -0.95f || mTarget.y > 0.95f) {
        //    mTarget = saveTarget;
        //}
    }
    // 左右转头（绕局部坐标系的Y轴旋转）
    void Yaw(float rad)
    {
        // 绕Y轴旋转
        XMVECTOR newTarget = XMVector4Transform(XMLoadFloat4(&mTarget), XMMatrixRotationY(rad));
        XMStoreFloat4(&mTarget, newTarget);
        // 右方向也要更新
        XMVECTOR newRightAxis = XMVector4Transform(XMLoadFloat4(&mRightAxis), XMMatrixRotationY(rad));
        XMStoreFloat4(&mRightAxis, newRightAxis);
    }
    // 跳跃
    void Jump()
    {
        // 加锁，不允许在空中二次跳跃或浮空
        if (up_down_lock == 0) {
            up_down_lock = 1;
            up_down = 1;
        }
    }
    // 检查跳跃状态，minHeight为跳跃前高度，maxHeight为跳跃的最大高度
    void CheckJump(float d, float minHeight, float maxHeight)
    {
        if (mPosition.y < maxHeight && up_down == 1) {
            mPosition.y += d;
        }
        else if (mPosition.y >= maxHeight && up_down == 1) {
            up_down = -1;
        }
        if (mPosition.y > minHeight && up_down == -1) {
            mPosition.y -= d;
        }
        else if (mPosition.y <= minHeight && up_down == -1) {
            up_down = 0;
            up_down_lock = 0;
            mPosition.y = minHeight;
        }
    }
};
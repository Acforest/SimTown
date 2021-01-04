#pragma once
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
// ��һ�˳����
class FirstPersonCamera
{
private:
    XMFLOAT4 mPosition;     // λ��
    XMFLOAT4 mTarget;       // ����λ��
    XMFLOAT4 mUpAxis;       // �Ϸ��򣨾ֲ�����ϵ��+Y�ᣩ
    XMFLOAT4 mRightAxis;    // �ҷ��򣨾ֲ�����ϵ��+X�ᣩ
    int up_down;            // ��Ծ״̬��0��ʾ�ڵ��棬1��ʾ��������-1��ʾ���½���
    int up_down_lock;       // ��Ծ״̬��
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
    // �����ƶ�
    void MoveLeftRight(float d)
    {
        float normalize_Length = mRightAxis.x * mRightAxis.x + mRightAxis.z * mRightAxis.z;
        float normalize_X = mRightAxis.x / normalize_Length;
        float normalize_Z = mRightAxis.z / normalize_Length;
        mPosition.x += normalize_X * d;
        mPosition.z += normalize_Z * d;
    }
    // ǰ���ƶ�
    void MoveForwardBack(float d)
    {
        float normalize_Length = mTarget.x * mTarget.x + mTarget.z * mTarget.z;
        float normalize_X = mTarget.x / normalize_Length;
        float normalize_Z = mTarget.z / normalize_Length;
        mPosition.x += normalize_X * d;
        mPosition.z += normalize_Z * d;
    }
    // ̧ͷ��ͷ���ƾֲ�����ϵ��X����ת��
    void Pitch(float rad)
    {
        // �ƾֲ�����ϵ��X����ת
        XMFLOAT4 saveTarget = mTarget;
        XMVECTOR newTarget = XMVector4Transform(XMLoadFloat4(&mTarget), XMMatrixRotationAxis(XMLoadFloat4(&mRightAxis), rad));
        XMStoreFloat4(&mTarget, newTarget);
        // ��ֹ���������
        //if (mTarget.y < -0.95f || mTarget.y > 0.95f) {
        //    mTarget = saveTarget;
        //}
    }
    // ����תͷ���ƾֲ�����ϵ��Y����ת��
    void Yaw(float rad)
    {
        // ��Y����ת
        XMVECTOR newTarget = XMVector4Transform(XMLoadFloat4(&mTarget), XMMatrixRotationY(rad));
        XMStoreFloat4(&mTarget, newTarget);
        // �ҷ���ҲҪ����
        XMVECTOR newRightAxis = XMVector4Transform(XMLoadFloat4(&mRightAxis), XMMatrixRotationY(rad));
        XMStoreFloat4(&mRightAxis, newRightAxis);
    }
    // ��Ծ
    void Jump()
    {
        // �������������ڿ��ж�����Ծ�򸡿�
        if (up_down_lock == 0) {
            up_down_lock = 1;
            up_down = 1;
        }
    }
    // �����Ծ״̬��minHeightΪ��Ծǰ�߶ȣ�maxHeightΪ��Ծ�����߶�
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
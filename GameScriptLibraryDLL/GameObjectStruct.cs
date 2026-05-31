using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL
{
    public enum GameObjectMemberType
    {
        GameObjectVarUnknown,
        GameObjectVarInt,
        GameObjectVarUint,
        GameObjectVarFloat,
        GameObjectVarIvec2,
        GameObjectVarIvec3,
        GameObjectVarIvec4,
        GameObjectVarVec2,
        GameObjectVarVec3,
        GameObjectVarVec4,
        GameObjectVarMat2,
        GameObjectVarMat3,
        GameObjectVarMat4,
        GameObjectVarBool
    };

    public unsafe struct GameObjectVariableDLL
    {
        public IntPtr VariableName;
        public byte* ValuePtr;
        public size_t ValueByteSize;
        public GameObjectMemberType MemberTypeEnum;
        public bool ConstVariable;

        public T* GetAs<T>()
        {
            if (typeof(T) == typeof(int)) return (T*)ValuePtr;
            if (typeof(T) == typeof(float)) return (T*)ValuePtr;
            return (T*)ValuePtr;
        }
    }

    public unsafe struct GameObjectVariable<T>
    {
        public string VariableName;
        public T Value;
        public size_t ValueByteSize;
        public GameObjectMemberType MemberTypeEnum;
        public bool ConstVariable;

        public GameObjectVariable()
        {
        }

    }

    public struct GameObjectStruct<T>
    {
        Dictionary<String, GameObjectVariable<T>> GameObjectVariableMap;
    }
}

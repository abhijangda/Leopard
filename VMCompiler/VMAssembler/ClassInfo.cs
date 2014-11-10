using System;
using System.Collections.Generic;

namespace VMAssembler
{
	public abstract class Info
	{
		public abstract List<byte> toBinary ();
	}

	public class MemberInfo : Info
	{
		public enum AccessSpecifer
		{
			Public,
			Private,
			Protected
		}

		public bool isStatic {get; private set;}
		public string name {get; private set;}
		public string type {get; private set;}
		public AccessSpecifer accessSpec {get; private set;}

		public MemberInfo (string name, string type, bool isStatic, AccessSpecifer accessSpec)
		{
			this.name = name;
			this.type = type;
			this.isStatic = isStatic;
			this.accessSpec = accessSpec;
		}

		public static AccessSpecifer StringToAccessSpecifier (string accessSpec)
		{
			foreach (string s in Enum.GetNames (typeof (MethodInfo.AccessSpecifer)))
			{
				if (s.ToLower () == accessSpec.ToLower ())
				{
					return (MethodInfo.AccessSpecifer)Enum.Parse (typeof (MethodInfo.AccessSpecifer), accessSpec);
				}
			}

			return MethodInfo.AccessSpecifer.Private;
		}

		protected byte getAccessSpecifierInt ()
		{
			if (accessSpec == AccessSpecifer.Private)
			{
				return 0;
			}
			else if (accessSpec == AccessSpecifer.Public)
			{
				return 1;
			}
			else if (accessSpec == AccessSpecifer.Protected)
			{
				return 2;
			}

			return 3;
		}

		protected byte getIsStaticInt ()
		{
			if (isStatic)
				return 1;
			return 0;
		}

		public override List<byte> toBinary ()
		{
			List<byte> listByte = new List<byte> ();

			listByte.Add ((byte)((getAccessSpecifierInt () << 1) + getIsStaticInt ()));
			listByte.Add ((byte)type.Length);
			listByte.AddRange (VMAssembler.ConvertStringToBytes (type));
			listByte.Add ((byte)name.Length);
			listByte.AddRange (VMAssembler.ConvertStringToBytes (name));

			return listByte;
		}
	}

	public class MethodCode : Info
	{
		public class Instruction : List<byte>
		{
		}

		public class Local : Info
		{
			int number;
			int size;
			string type;

			public Local (int n, int size, string type)
			{
				number = n;
				this.size = size;
				this.type = type;
			}

			public override List<byte> toBinary ()
			{
				List<byte> listBytes = new List<byte> ();

				listBytes.AddRange (VMAssembler.ConvertToByteList (number));
				listBytes.Add ((byte)type.Length);
				listBytes.AddRange (VMAssembler.ConvertStringToBytes (type));
				listBytes.AddRange (VMAssembler.ConvertToByteList (size));

				return listBytes;
			}
		}

		int totalLocals;
		List<Local> listLocals;
		List<Instruction> listInstructions;

		public MethodCode (int totalLocals, List<Local> listLocals, List<Instruction> listInstructions)
		{
			this.totalLocals = totalLocals;
			this.listLocals = listLocals;
			this.listInstructions = listInstructions;
		}

		public override List<byte> toBinary ()
		{
			List<byte> listBytes = new List<byte> ();

			listBytes.AddRange (VMAssembler.ConvertToByteList (totalLocals));
			foreach (Local local in listLocals)
			{
				listBytes.AddRange (local.toBinary ());
			}

			listBytes.AddRange (VMAssembler.ConvertToByteList ((long)listInstructions.Count));
			foreach (Instruction instr in listInstructions)
			{
				listBytes.AddRange (instr);
			}

			return listBytes;
		}
	}

	public class MethodInfo : MemberInfo 
	{
		List<string> paramTypes;
		MethodCode code;

		public MethodInfo (string name, string type, bool isStatic, AccessSpecifer accessSpec, List<string> paramTypes,
			MethodCode codePtr) : 
		base (name, type, isStatic, accessSpec)
		{
			this.paramTypes = paramTypes;
			code = codePtr;
		}

		public override List<byte> toBinary ()
		{
			List<byte> listByte = base.toBinary ();

			listByte.Add ((byte)paramTypes.Count);

			foreach (string s in paramTypes)
			{
				listByte.Add ((byte)s.Length);
				listByte.AddRange (VMAssembler.ConvertStringToBytes (s));
			}

			listByte.AddRange (code.toBinary ());

			return listByte;
		}
	}

	public class ClassInfo : Info
	{
		public string name {get; private set;}
		int size;
		string parent;
		byte n_members;
		byte n_methods;
		List<MethodInfo> listMethodInfo;
		List<MemberInfo> listMemberInfo;

		public ClassInfo (string name, int size, string parent, byte n_members, byte n_methods,
			List<MethodInfo> listMethodInfo, List<MemberInfo> listMemberInfo)
		{
			this.name = name;
			this.size = size;
			this.parent = parent;
			this.n_members = n_members;
			this.n_methods = n_methods;
			this.listMemberInfo = listMemberInfo;
			this.listMethodInfo = listMethodInfo;
		}

		public override List<byte> toBinary ()
		{
			List<byte> listbytes = new List<byte> ();

			listbytes.Add ((byte)name.Length);
			listbytes.AddRange (VMAssembler.ConvertStringToBytes (name));
			listbytes.AddRange (VMAssembler.ConvertToByteList (size));
			listbytes.Add ((byte)parent.Length);
			listbytes.AddRange (VMAssembler.ConvertStringToBytes (parent));

			listbytes.Add ((byte)listMemberInfo.Count);
			foreach (MemberInfo minfo in listMemberInfo)
				listbytes.AddRange (minfo.toBinary ());

			listbytes.Add ((byte)listMethodInfo.Count);
			foreach (MethodInfo minfo in listMethodInfo)
				listbytes.AddRange (minfo.toBinary ());

			return listbytes;
		}
	}
}
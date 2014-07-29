package registers;

public abstract class Register 
{
	public enum RegisterType
	{
		Int64Register,
		Int32Register,
		Int16Register,
		Int8Register,
		DoubleRegister,
		FloatRegister,
		ASCIICharacterRegister,
		UTF16CharacterRegister,
		ObjectReferenceRegister,
	}
	
	public byte code;
	public abstract int getSize ();
	public RegisterType type;
}

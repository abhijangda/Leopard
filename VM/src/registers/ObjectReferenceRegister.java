package registers;

public class ObjectReferenceRegister extends Int64Register
{
	public ObjectReferenceRegister (byte _code) 
	{
		super (_code);
		type = RegisterType.ObjectReferenceRegister;
	}
}

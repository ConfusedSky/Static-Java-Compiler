public class Test {

	public static void printInt(int value) {
		System.out.println(value);
	}
	public static void main(String[] args) {
		int x = 0;
		try
		{
			printInt(x);
		}
		catch(ArithmeticException e)
		{
			System.out.println("There was an error");
		}
		finally
		{
			System.out.println("Always called");
		}
	}

}

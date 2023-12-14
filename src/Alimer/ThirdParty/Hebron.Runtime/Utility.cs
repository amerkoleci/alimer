namespace Hebron.Runtime
{
	internal class Utility
	{
		public static T[][] CreateArray<T>(int d1, int d2)
		{
			var result = new T[d1][];
			for (int i = 0; i < d1; i++)
			{
				result[i] = new T[d2];
			}

			return result;
		}
	}
}

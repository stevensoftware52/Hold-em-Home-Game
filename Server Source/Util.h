#include "stdafx.h"

typedef struct Vector2
{
	Vector2(float a = 0, float b = 0):
		x(a), y(b) {}

	float x;
	float y;

} VEC2_STRUCT;

class Util
{
	public:

		template<class Iterator>
		static Iterator Unique(Iterator first, Iterator last)
		{
			while (first != last)
			{
				Iterator next(first);
				last = std::remove(++next, last, *first);
				first = next;
			}

			return last;
		}

		static int randomNumber(int min, int max)
		{
			return rand() % max + min;
		}

		static float dist(float x1, float y1, float x2, float y2)
		{
			if (x1 == x2 && y1 == y2)
				return 0;

			return sqrt(pow((x2-x1),2)+pow((y2-y1), 2));
		}

		static float round(float r)
		{
			return (r > 0.0f) ? floor(r + 0.5f) : ceil(r - 0.5f);
		}

		static float dist(Vector2 one, Vector2 two)
		{
			return dist(one.x, one.y, two.x, two.y);
		}

		static float twoDec(float val)
		{
			return floor(val * 100) / 100;
		}

		static Vector2 extrude(float x1, float y1, float x2, float y2, float distance)
		{
			Vector2 result(x1, y1);
    
			if (distance && dist(x1, y1, x2, y2))
			{
				result.x = x1 - distance*(x1-x2)/sqrt((float)(((x1-x2)*(x1-x2))+((y1-y2)*(y1-y2))));
				result.y = y1 - distance*(y1-y2)/sqrt((float)(((x1-x2)*(x1-x2))+((y1-y2)*(y1-y2))));
			}

			return result;
		}

		static unsigned int bufferAddInt(char *buffer, int parameter)
		{
			*(int *)(buffer) = parameter;
			return (unsigned int)sizeof(parameter);
		}

		static unsigned int bufferAddUInt(char *buffer, unsigned int parameter)
		{
			*(unsigned int *)(buffer) = parameter;
			return (unsigned int)sizeof(parameter);
		}

		static unsigned int bufferAddFloat(char *buffer, float parameter)
		{
			*(float *)(buffer) = parameter;
			return (unsigned int)sizeof(parameter);
		}

		static unsigned int bufferAddChar(char *buffer, char parameter)
		{
			*(char *)(buffer) = parameter;
			return (unsigned int)sizeof(parameter);
		}

		static unsigned int bufferAddUShort(char *buffer, unsigned short parameter)
		{
			*(unsigned short *)(buffer) = parameter;
			return (unsigned int)sizeof(parameter);
		}

		static unsigned int bufferAddBool(char *buffer, bool parameter)
		{
			*(bool *)(buffer) = parameter;
			return (unsigned int)sizeof(parameter);
		}

		static unsigned int bufferAddUINT8(char *buffer, uint8 parameter)
		{
			*(uint8 *)(buffer) = parameter;
			return (unsigned int)sizeof(parameter);
		}

		static unsigned int bufferAddString(char *buffer, const char *parameter)
		{
			for (unsigned int i = 0; i < strlen(parameter); i++)
			{
				*(char *)(buffer + (i * sizeof(char))) = parameter[i];
			}
			return (unsigned int)strlen(parameter) * sizeof(char);
		}

		static int sendBytes(SOCKET sock, char *buffer, int size)
		{
			int result = size;

			// Send the msg until complete
			for (volatile unsigned int amountSent = 0, maxSend = size, amountLeft = size; amountSent < maxSend && result > 0;)
			{
				result = send(sock, buffer + amountSent, amountLeft, 0); 

				if (result == SOCKET_ERROR)
					return result;

				amountLeft -= result;
				amountSent += result;
			}

			return size;
		}

		static int sendIntAsBytes(SOCKET sock, DWORD value)
		{
			char buffer[4];
			bufferAddInt(buffer, value);
			return sendBytes(sock, buffer, 4);
		}

		static int recvBytes(SOCKET sock, char *buffer, int size)
		{        
			int result = 0;

			for (int i = 0; i < size; ++i)
			{
				int ret = recv(sock, buffer + i, 1, 0);

				if (ret <= 0)
					return ret;

				result += ret;
			}

			if (result != size)
				printf("ERROR: recvBytes mis matched\n");

			return result;
		}
};
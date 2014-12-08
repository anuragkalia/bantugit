#ifndef PQUEUE_H
#define PQUEUE_H

#include "debug.h"
#include "terminal.h"

extern "C" void kernel_main();

namespace myos
{
	template<class T, size_t N>
	class pqueue
	{
		friend void ::kernel_main();
		T arr[N];
		size_t sz;
		int front, back;

		typedef bool(*less_func)(T e1, T e2);

		less_func less;

		void swap(T &e1, T &e2)
		{
			//magic_breakpoint_msg("In swap\n");
			T temp = e1;
			e1 = e2;
			e2 = temp;
			//magic_breakpoint_msg("Out swap\n");
		}

		void sort_insert(size_t b, size_t e)
		{
			if (b <= e)
			{

				//magic_breakpoint_msg("b <= e\n");

				for (size_t i = e; i > b; i--)
				{
					//magic_breakpoint_msg("Before using 'less' - 1\n");
					if (less(arr[i], arr[i - 1]))
					//if (arr[i] < arr[i - 1])
					{
						swap(arr[i - 1], arr[i]);
					}
					else break;
				}
			}
			else
			{
				//magic_breakpoint_msg("b > e\n");
				sort_insert(0, e);
				//magic_breakpoint_msg("Before using 'less' - 2\n");
				if (less(arr[0], arr[N - 1]))
				//if (arr[0] < arr[N - 1])
				{
					swap(arr[0], arr[N - 1]);
				}
				else
				{
					//magic_breakpoint_msg("Out sort_insert()\n");
					return;
				}
				sort_insert(b, N - 1);
			}
			//magic_breakpoint_msg("Out sort_insert()\n");
		}

		void sort_all() { magic_breakpoint_msg("In sort_all(): SHOULD NOT BE HERE\n"); }

	public:
		
		void print() const
		{
			size_t count = 0;
			terminal_writestring("pqueue: ");
			for (int i = front; count < sz; i = (i + 1) % N, count++)
			{
				terminal_writenumber(arr[i]);
				terminal_writestring(", ");
			}
			terminal_putchar('\n');
		}
		
		pqueue(less_func L)
			: sz(0)
		{
			//magic_breakpoint_msg("In pqueue ctr()\n");
			less = L;
			magic_breakpoint();
		}

		void change_element_ordering(less_func L)
		{
			//magic_breakpoint_msg("In change_element_ordering() : CANNOT BE HERE\n");
			less = L;
			sort_all();
		}

		bool enqueue(T elem)
		{
			//terminal_clear();
			//log(OK, "enqueue\n");

			if (sz == N)
			{
				magic_breakpoint_msg("overflow");
				//magic_breakpoint_msg("Out enqueue\n");
				return false;
			}
			else
			{

				if (sz == 0)
				{
					front = back = 0;
				}
				arr[back] = elem;
				//magic_breakpoint_msg("Before entering sort_insert()\n");
				sort_insert(front, back);
				//magic_breakpoint_msg("Back in enqueue from sort_insert()\n");
				back = (back + 1) % N;
				sz++;
				//print();
				//magic_breakpoint_msg("Out enqueue\n");
				return true;
			}
			
		}

		T dequeue()
		{
			//log(OK, "dequeue\n");
			if (sz > 0)
			{
				T out = arr[front];
				front = (front + 1) % N;
				sz--;
				return out;
			}
			else
			{
				return T(0);
			}
			//print();
		}

		bool is_empty() const
		{
			return (sz == 0);
		}

		int size() const { return sz; }
	};
}
#endif //PQUEUE_H
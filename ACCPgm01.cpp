#include <iostream>
#include <string>		// For size_t
#include <new>			// For bad_alloc

using namespace std;

// Prototypes
double **Create2DArray(size_t rows, size_t cols);
int GetRows(double **ara);
int GetCols(double **ara);
void Delete2DArray(double **ara);

// Main
int main(void)
{
	double **dAra = Create2DArray(12, 9);	// Create an array with 12 rows and 9 columns.

	// Display an error message if the allocation failed.  !dAra is the same thing as dAra == NULL
	if (!dAra)
	{
		cerr << "Could not allocate memory." << endl;
		return 1;
	}

	cout << "dAra created." << endl;
	cout << "Rows: " << GetRows(dAra) << endl;	// Should display 12
	cout << "Cols: " << GetCols(dAra) << endl;	// Should display 9

	// Populate the array
	// Not the most efficient way of doing it, but it shows
	// that my functions work.
	for (int row = 0; row < GetRows(dAra); row++)
	{
		for (int col = 0; col < GetCols(dAra); col++)
		{
			dAra[row][col] = (row * 10.0) + (double)col;
		}
	}

	// Show the array.
	for (int row = 0; row < GetRows(dAra); row++)
	{
		for (int col = 0; col < GetCols(dAra); col++)
		{
			cout << dAra[row][col] << ' ';
		}
		cout << endl;
	}

	// Free up the memory used by our array.
	Delete2DArray(dAra);
	return 0;
}

// Create2DArray //////////////////////////////////////////
double **Create2DArray(size_t rows, size_t cols)
{
	double **ara;

	// I'm doing a try-catch block in case the allocation fails.  If the allocation fails,
	// the catch (bad_alloc) line will detect it.
	try
	{
		// Basically, when we're trying to treat a pointer to a pointer as a 2-dimensional
		// array of contiguous memory, we have to do the following:
		//
		// 1) Dynamically allocate an array of pointers to have access to each row.
		//    If I want to dynamically allocate an array of pointers, I would write
		//    code along these lines.
		//
		//    double **pointer_to_pointer;
		//    pointer_to_pointer = new double *[rows];	// Yes, the asterisk is there on purpose, because each of these elements must hold a pointer.
		//
		// 2) Allocate space for a buffer of contiguous memory, equal to the total number of
		//    elements that we need.
		//
		//    double *our_buffer;
		//    our_buffer = new double[rows * cols];
		//
		// 3) Populate our array of pointers with addresses of different positions in our buffer.
		//    Usually these chunks will be of equal length.
		//
		//    // Not the most efficient way, but the most straightforward.
		//    for (int index = 0; index < rows; index++)
		//    {
		//       // The ampersand is required to get the ADDRESS of the element in
		//       // our_buffer.  Remember, dynamically allocated memory can be treated
		//       // exactly like an array.
		//
		//       pointer_to_pointer[index] = &our_buffer[index * cols];
		//    }
		//

		// If the user tries to allocate with zero rows or zero columns, then
		// immediately put a stop to it.  Zero by anything instantly becomes an
		// array of size zero.
		if ((rows == 0) || (cols == 0))
		{
			throw bad_alloc();	// This continues execution at the catch (bad_alloc) line.
		}

		// Allocate an array of pointers with the required number of rows.
		//
		// The + 2 is there because I'm requesting two more spots.  I'm taking advantage
		// of the fact that ints and pointers always take up the same amount of memory,
		// and I'm going to use the first two elements to store ints.  Store ints, you say?
		// But aren't I allocating an array of pointers to doubles?  Yes, that is true, but
		// if you think about it, an int is a number, and a pointer, although an address,
		// is essentially a number.  If I cast it to a pointer, I'm basically making the
		// compiler "think" that it's dealing with a pointer, when it's really not.
		//
		ara = new double *[rows + 2];		// Watch what the extra 2 bytes are for.

		// Store the array information
		// The cast to (double *) is required to make the compiler think
		// that it's dealing with a pointer.  If I don't do this, it simply
		// will not compile.
		//
		ara[0] = (double *)rows;	// Store the rows in the first element
		ara[1] = (double *)cols;	// Store the columns in the second element

		// Skip past the first two elements.
		//
		// In this case, the compiler knows that I'm dealing with a pointer to a pointer to
		// doubles.  By adding two, I'm basically saying to "move the beginning ahead by the size
		// of two pointers to doubles."  This is known as pointer arithmetic.  For every 1,
		// the compiler automatically treats it as the size of the type something is pointing TO.
		//
		ara += 2;		// Basically, just view this as skipping two elements ahead.

		// Since I'm skipping two ahead, when time comes to free the array, I have to remember
		// to move back by two elements.  For now, the information about the size of the array
		// is safely tucked away without the user's knowledge.  For now, I can treat the first
		// element of ara as if it's the beginning of my array of rows.
		//
		// So, let's allocate space for the buffer.
		//
		ara[0] = new double[rows * cols];	// Remember, ara[0] is the TRUE pointer to the allocated buffer of memory.

		// Initialize the pointers
		//
		// ara[-2] is the number of rows
		// ara[-1] is the number of columns (but we can safely ignore this fact)
		// ara[-1] simply means "the spot in memory one element BEFORE the start of the array,"
		// and YES, this can actually happen.
		//
		// ara[0] is the first row
		// ara[1] is the second row
		// ara[2] is the third row (and so on)
		//
		// We are going to start at the second row of the array, since by allocating the
		// buffer at the first row, the first row already knows its address by default.
		//
		// The size of each row is determined by the number of columns in the array,
		// so to get the address of the next row, we add by the number of columns.
		//
		// size_t is just an integral type that is intended for sizes of memory.  It's the
		// same thing as unsigned int.
		//
		for (size_t index = 1, offset = cols; index < rows; index++, offset += cols)
		{
			// Remember, ara[0] is the address to the first element of our buffer.
			//
			// If offset == cols, then ara[0][offset] is the first ELEMENT of our
			// second row, or, the first COLUMN of our second row, which is a double.
			//
			// Now, consider this.
			//
			// ara is a pointer, to a pointer to a double.
			// ara[index] is a pointer to a double.
			// ara[index][anything] is simply a double.
			//
			// Notice how everytime I throw in a [], the word "pointer" goes away.
			//
			// In the following line of code, ara[0][offset] is simply a double.  However,
			// ara[index] is a pointer to a double.  At the same time, ara[index] is a pointer
			// to the STARTING COLUMN of our row.
			//
			// With each iteration of our loop, ara[0][offset] becomes the starting column of
			// a new row.  In order to get a pointer to this, we have to prefix it with an
			// ampersand, which simply means "get the address of this."
			//
			ara[index] = &ara[0][offset];
		}

		// Return the newly created array
		return ara;
	}

	// Allocation errors go here.
	catch (bad_alloc)
	{
		return NULL;			// Allocation failed.  Return a NULL pointer.
	}
}

// GetRows ////////////////////////////////////////////////
int GetRows(double **ara)
{
	// The expression (ara - 2) means give me the address that is two elements back.
	// The asterisk in front means to get the data there.
	//
	return (int)(*(ara - 2));	// Remember, the number of rows is two elements behind our actual array of rows.
}

// GetCols ////////////////////////////////////////////////
int GetCols(double **ara)
{
	// Refer to GetRows().
	return (int)(*(ara - 1));	// Refer to the code in Create2DArray().
}

// Delete2DArray //////////////////////////////////////////
void Delete2DArray(double **ara)
{
	delete[] *ara;		// We allocated an array at ara[0], remember?  *ara means the same thing as ara[0].
	ara -= 2;			// Move two elements back.
	delete[] ara;		// Free the array of rows.
}
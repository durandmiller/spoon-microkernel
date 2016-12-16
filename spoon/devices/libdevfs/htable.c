#include <smk.h>
#include <devfs.h>

/*  hash table implementation for fast-lookups in the kernel.  */ 



struct devfs_htable *devfs_init_htable( int size, 
				 				int percent, 
								hash_key_func key,
								hash_compare_func compare
							  )
{
	int i;
	struct devfs_htable *table = (struct devfs_htable*)smk_malloc(sizeof(struct devfs_htable));
	if ( table == NULL ) return NULL;
	

	  table->percent = percent;
	  table->size    = size;
	  table->key     = key;
	  table->compare = compare;
	  table->total   = 0;

	  if ( table->size <= 0 )  table->size = 32;

	  table->table = (void**)smk_malloc(table->size * sizeof(void**));

		for ( i = 0; i < table->size; i++ )
				table->table[i] = NULL;


	return table;
}


int devfs_delete_htable( struct devfs_htable* table )
{
	smk_free( table->table );
	smk_free( table );
	return 0;
}

// -------------------------------------------------------------


int devfs_htable_rehash( struct devfs_htable* table, int new_size )
{
	// Allocate the new table..
	void **newTable = (void**)smk_malloc(sizeof(void**) * new_size);
	if ( newTable == NULL ) return -1;		// Out of memory.


	void **oldTable = table->table;
	int oldSize     = table->size;
	int i;

	for ( i = 0; i < new_size; i++) newTable[i] = NULL;

	table->size = new_size;
	table->total = 0;
	table->table = newTable;


	for ( i = 0; i < oldSize; i++ )
		if ( oldTable[i] != NULL ) devfs_htable_insert( table, oldTable[i] );

	smk_free( oldTable );
	return 0;
}

// -------------------------------------------------------------

int devfs_htable_insert( struct devfs_htable* table, void *data )
{
	int current_percent;
	int position;
	int key;
	int i;
			
	if ( table->total == table->size ) return -1;

	current_percent = ((table->total + 1) * 100) / table->size;
 
	if ( current_percent > table->percent ) 
				devfs_htable_rehash( table, table->size * 2 );
	

	key = table->key( data );
	if ( key < 0 ) key = -key;


	for ( i = 0; i < table->size; i++ )
	{
		position = ( i + key ) % table->size;

		if ( table->table[position] == NULL )
		{
			table->table[position] = data;
			table->total += 1;
			break;
		}
	}
	
	return 0;
}


int devfs_htable_remove( struct devfs_htable* table, void *data )
{
	int current_percent;
	int position;
	int key;
	int i;
			
	if ( table->total == 0 ) return -1;

	current_percent = ((table->total - 1) * 100) / table->size;
   
	if ( (current_percent< (table->percent / 4 )) && (table->size > 32) )
			devfs_htable_rehash( table, table->size / 2 );
	

	key = table->key( data );
	if ( key < 0 ) key = -key;

	for ( i = 0; i < table->size; i++ )
	{
		position = ( i + key ) % table->size;

		if ( table->table[position] == data )
		{
			table->table[position] = NULL;
			table->total -= 1;
			break;
		}
	}
	
	return 0;

}

void *devfs_htable_retrieve( struct devfs_htable* table, void *sample )
{
	int position;
	int key;
	int i;
			
	if ( table->total == 0 ) return NULL;


	key = table->key( sample );
	if ( key < 0 ) key = -key;

	for ( i = 0; i < table->size; i++ )
	{
		position = ( i + key ) % table->size;

		if ( table->table[position] != NULL )
			if ( table->compare( table->table[position], sample ) == 0  )
				return table->table[position];


	}
	
	return NULL;
}



void *devfs_htable_get( struct devfs_htable* table, int num )
{
	int i;
	int count = 0;
			
	if ( table->total == 0 ) return NULL;
	if ( num >= table->total ) return NULL;


	for ( i = 0; i < table->size; i++ )
	{
		if ( table->table[i] != NULL )
			if ( count++ == num ) return table->table[i];
	}
	
	return NULL;
}






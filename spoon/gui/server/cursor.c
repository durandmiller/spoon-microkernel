#include "cursor.h"
#include "base/reserves.h"


pixel_t cursor_up_bitmap[CURSOR_HEIGHT][CURSOR_WIDTH] = 
{  
	{ 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, TRANSPARENT, TRANSPARENT  },
	{ 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, TRANSPARENT, TRANSPARENT, TRANSPARENT  },
	{ 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT  },
	{ 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT  },
	{ 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, TRANSPARENT, TRANSPARENT, TRANSPARENT  },
	{ 0xFFFFFF, TRANSPARENT, TRANSPARENT, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, TRANSPARENT, TRANSPARENT  },
	{ TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, TRANSPARENT  },
	{ TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, 0xFFFFFF, 0xFFFFFF, TRANSPARENT  } };

pixel_t cursor_down_bitmap[CURSOR_HEIGHT][CURSOR_WIDTH] = 
{  
	{ 0xFFAA88, 0xFFAA88, 0xFFAA88, 0xFFAA88, 0xFFAA88, 0xFFAA88, TRANSPARENT, TRANSPARENT  },
	{ 0xFFAA88, 0xFFAA88, 0xFFAA88, 0xFFAA88, 0xFFAA88, TRANSPARENT, TRANSPARENT, TRANSPARENT  },
	{ 0xFFAA88, 0xFFAA88, 0xFFAA88, 0xFFAA88, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT  },
	{ 0xFFAA88, 0xFFAA88, 0xFFAA88, 0xFFAA88, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT  },
	{ 0xFFAA88, 0xFFAA88, 0xFFAA88, 0xFFAA88, 0xFFAA88, TRANSPARENT, TRANSPARENT, TRANSPARENT  },
	{ 0xFFAA88, TRANSPARENT, TRANSPARENT, 0xFFAA88, 0xFFAA88, 0xFFAA88, TRANSPARENT, TRANSPARENT  },
	{ TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, 0xFFAA88, 0xFFAA88, 0xFFAA88, TRANSPARENT  },
	{ TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, 0xFFAA88, 0xFFAA88, TRANSPARENT  } };



pixel_t buffer_bitmap[CURSOR_HEIGHT][CURSOR_WIDTH] = 
{  
	{ TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT  },
	{ TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT  },
	{ TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT  },
	{ TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT  },
	{ TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT  },
	{ TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT  },
	{ TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT  },
	{ TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT  } };



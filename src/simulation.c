#include <simulation.h>

float get_random_SW_INLET_temp(int r)
{
	srand((unsigned int)time(NULL) + r);
	int i;
	float low_range = 60.0;
	float upper_range = 60.3;
	
	float scale = upper_range - low_range;
	
	return ((float)rand()/(float)(RAND_MAX)) * scale + low_range;
}

float get_random_SW_OUTLET_temp(int r)
{
	srand((unsigned int)time(NULL) + r);
	int i;
	float low_range = 75.0;
	float upper_range = 75.3;
	
	float scale = upper_range - low_range;
	
	return ((float)rand()/(float)(RAND_MAX)) * scale + low_range;
}

float get_random_RHR_INLET_temp(int r)
{
	srand((unsigned int)time(NULL) + r);
	int i;
	float low_range = 90.0;
	float upper_range = 91.4;
	
	float scale = upper_range - low_range;
	
	return ((float)rand()/(float)(RAND_MAX)) * scale + low_range;
}

float get_random_RHR_OUTLET_temp(int r)
{
	srand((unsigned int)time(NULL) + r);
	int i;
	float low_range = 76.0;
	float upper_range = 76.3;
	
	float scale = upper_range - low_range;
	
	return ((float)rand()/(float)(RAND_MAX)) * scale + low_range;
}


float get_random_SW_flow(int r)
{
	srand((unsigned int)time(NULL) + r);
	int i;
	float low_range = 100.0;
	float upper_range = 100.3;
	
	float scale = upper_range - low_range;
	
	return ((float)rand()/(float)(RAND_MAX)) * scale + low_range;
}

float get_random_RHR_flow(int r)
{
	srand((unsigned int)time(NULL) + r);
	int i;
	float low_range = 100.0;
	float upper_range = 100.3;
	
	float scale = upper_range - low_range;
	
	return ((float)rand()/(float)(RAND_MAX)) * scale + low_range;
}



int setup_simulation_data_for_current_cm(bridge *bridge_data){
		
		int i;
		
		for (i = 0; i <bridge_data->cm[bridge_data->index_cm].size_sen; i++)
		{
			if (strcmp(bridge_data->cm[bridge_data->index_cm].sen[i].addr, "02A002") == 0)
			{
				bridge_data->cm[bridge_data->index_cm].sen[i].data = get_random_SW_INLET_temp(1);
			}
			else if (strcmp(bridge_data->cm[bridge_data->index_cm].sen[i].addr, "02A003") == 0)
			{
				bridge_data->cm[bridge_data->index_cm].sen[i].data = get_random_SW_INLET_temp(2);
			}
			else if (strcmp(bridge_data->cm[bridge_data->index_cm].sen[i].addr, "02A004") == 0)
			{
				bridge_data->cm[bridge_data->index_cm].sen[i].data = get_random_SW_INLET_temp(3);
			}
			else if (strcmp(bridge_data->cm[bridge_data->index_cm].sen[i].addr, "02A005") == 0)
			{
				bridge_data->cm[bridge_data->index_cm].sen[i].data = get_random_SW_INLET_temp(4);
			}
			else if (strcmp(bridge_data->cm[bridge_data->index_cm].sen[i].addr, "02A008") == 0)
			{
				bridge_data->cm[bridge_data->index_cm].sen[i].data = get_random_SW_OUTLET_temp(1);
			}
			else if (strcmp(bridge_data->cm[bridge_data->index_cm].sen[i].addr, "02A009") == 0)
			{
				bridge_data->cm[bridge_data->index_cm].sen[i].data = get_random_SW_OUTLET_temp(2);
			}
			else if (strcmp(bridge_data->cm[bridge_data->index_cm].sen[i].addr, "02A010") == 0)
			{
				bridge_data->cm[bridge_data->index_cm].sen[i].data = get_random_SW_OUTLET_temp(3);
			}
			else if (strcmp(bridge_data->cm[bridge_data->index_cm].sen[i].addr, "02A011") == 0)
			{
				bridge_data->cm[bridge_data->index_cm].sen[i].data = get_random_SW_OUTLET_temp(4);
			}
			else if (strcmp(bridge_data->cm[bridge_data->index_cm].sen[i].addr, "02A012") == 0)
			{
				bridge_data->cm[bridge_data->index_cm].sen[i].data = get_random_RHR_INLET_temp(5);
			}
			else if (strcmp(bridge_data->cm[bridge_data->index_cm].sen[i].addr, "02A013") == 0)
			{
				bridge_data->cm[bridge_data->index_cm].sen[i].data = get_random_RHR_INLET_temp(6);
			}
			else if (strcmp(bridge_data->cm[bridge_data->index_cm].sen[i].addr, "02A014") == 0)
			{
				bridge_data->cm[bridge_data->index_cm].sen[i].data = get_random_RHR_INLET_temp(7);
			}
			else if (strcmp(bridge_data->cm[bridge_data->index_cm].sen[i].addr, "02A015") == 0)
			{
				bridge_data->cm[bridge_data->index_cm].sen[i].data = get_random_RHR_INLET_temp(8);
			}
			else if (strcmp(bridge_data->cm[bridge_data->index_cm].sen[i].addr, "02A016") == 0)
			{
				bridge_data->cm[bridge_data->index_cm].sen[i].data = get_random_RHR_OUTLET_temp(9);
			}
			else if (strcmp(bridge_data->cm[bridge_data->index_cm].sen[i].addr, "02A017") == 0)
			{
				bridge_data->cm[bridge_data->index_cm].sen[i].data = get_random_RHR_OUTLET_temp(10);
			}
			else if (strcmp(bridge_data->cm[bridge_data->index_cm].sen[i].addr, "02A018") == 0)
			{
				bridge_data->cm[bridge_data->index_cm].sen[i].data = get_random_RHR_OUTLET_temp(11);
			}
			else if (strcmp(bridge_data->cm[bridge_data->index_cm].sen[i].addr, "02A019") == 0)
			{
				bridge_data->cm[bridge_data->index_cm].sen[i].data = get_random_RHR_OUTLET_temp(12);
			}
			else if (strcmp(bridge_data->cm[bridge_data->index_cm].sen[i].addr, "07A001") == 0)
			{
				bridge_data->cm[bridge_data->index_cm].sen[i].data = get_random_SW_flow(1);
			}
			else if (strcmp(bridge_data->cm[bridge_data->index_cm].sen[i].addr, "07A002") == 0)
			{
				bridge_data->cm[bridge_data->index_cm].sen[i].data = get_random_RHR_flow(2);
			}
			
		}
	
	
}
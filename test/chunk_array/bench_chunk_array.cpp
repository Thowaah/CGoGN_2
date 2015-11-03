
#include <core/container/chunk_array_container.h>

#define BLK_SZ 4096

using namespace cgogn;


/**
 * @brief The Vec3f class: just for the example
 */
class Vec3f
{
	float data_[3];
public:
	Vec3f() {}
	Vec3f(float x,float y, float z)
	{
		data_[0]=x;
		data_[1]=y;
		data_[2]=z;
	}
	static std::string CGoGNnameOfType()
	{
		return "Vec3f";
	}
};

const unsigned int NB_LINES = 20000000;

int test1()
{
	std::cout << "= TEST 1 = ref unsigned char" << std::endl;

	ChunkArrayContainer<BLK_SZ, 1, unsigned char> container;
	ChunkArray<BLK_SZ,int>* att1 = container.addAttribute<int>("entier");
	ChunkArray<BLK_SZ,float>* att2 = container.addAttribute<float>("reel");
	ChunkArray<BLK_SZ,Vec3f>* att3 = container.addAttribute<Vec3f>("Vec3f");


	for (unsigned int i=0;i<NB_LINES;++i)
		container.insertLines();



	for(unsigned int i=container.begin(); i!=container.end(); container.next(i))
	{
		(*att1)[i] = 1+int(i);
		(*att2)[i] = 3.0f + 0.1f*float(i);
		(*att3)[i] = Vec3f(float(i), float(i), float(i));
	}


	for (unsigned int j=0; j<40; ++j)
	{

		for (unsigned int i=0;i<NB_LINES/10;++i)
		{
			container.removeLines(j%2+1+i*10);
			container.removeLines(j%2+3+i*10);
			container.removeLines(j%2+8+i*10);
		}

		for (unsigned int i=0;i<3*NB_LINES/10;++i)
			container.insertLines();
	}

	std::cout << "---> OK" << std::endl;
	return 0;
}

int test2()
{
	std::cout << "= TEST 2 = ref bool" << std::endl;

	ChunkArrayContainer<BLK_SZ, 1, bool> container;
	ChunkArray<BLK_SZ,int>* att1 = container.addAttribute<int>("entier");
	ChunkArray<BLK_SZ,float>* att2 = container.addAttribute<float>("reel");
	ChunkArray<BLK_SZ,Vec3f>* att3 = container.addAttribute<Vec3f>("Vec3f");


	for (unsigned int i=0;i<NB_LINES;++i)
		container.insertLines();



	for(unsigned int i=container.begin(); i!=container.end(); container.next(i))
	{
		(*att1)[i] = 1+int(i);
		(*att2)[i] = 3.0f + 0.1f*float(i);
		(*att3)[i] = Vec3f(float(i), float(i), float(i));
	}


	for (unsigned int j=0; j<40; ++j)
	{

		for (unsigned int i=0;i<NB_LINES/10;++i)
		{
			container.removeLines(j%2+1+i*10);
			container.removeLines(j%2+3+i*10);
			container.removeLines(j%2+8+i*10);
		}

		for (unsigned int i=0;i<3*NB_LINES/10;++i)
			container.insertLines();
	}

	std::cout << "---> OK" << std::endl;
	return 0;
}



int test3()
{
	std::cout << "= TEST 3 = random bool cleaning" << std::endl;

	ChunkArrayContainer<BLK_SZ, 1, bool> container;
	ChunkArray<BLK_SZ,bool>* att1 = container.addAttribute<bool>("bools");

	for (unsigned int i=0;i<NB_LINES;++i)
		container.insertLines();

	for(unsigned int i=container.begin(); i!=container.end(); container.next(i))
	{
		att1->setVal(i,true);
	}

	for (unsigned int j=0; j<40; ++j)
	{
		for (unsigned int i=0;i<NB_LINES/2;++i)
		{
			att1->setFalse(i);
			att1->setFalse(NB_LINES-1-i);
		}
	}

	std::cout << "---> OK" << std::endl;
	return 0;
}


int test4()
{
	std::cout << "= TEST 4 = random bool cleaning with setFalseDirty" << std::endl;

	ChunkArrayContainer<BLK_SZ, 1, bool> container;
	ChunkArray<BLK_SZ,bool>* att1 = container.addAttribute<bool>("bools");

	for (unsigned int i=0;i<NB_LINES;++i)
		container.insertLines();

	for(unsigned int i=container.begin(); i!=container.end(); container.next(i))
	{
		att1->setVal(i,true);
	}

	for (unsigned int j=0; j<40; ++j)
	{
		for (unsigned int i=0;i<NB_LINES/2;++i)
		{
			att1->setFalseDirty(i);
			att1->setFalseDirty(NB_LINES-1-i);
		}
	}

	std::cout << "---> OK" << std::endl;
	return 0;
}






int main(int argc, char **argv)
{
	if (argc==1)
	{
		std::cout <<" PARAMETER: 1 (for unsigned int refs) / 2 (for bool refs)";
		return 1;
	}

	int c = atoi(argv[1]);
	switch(c)
	{
		case 1: test1();
			break;
		case 2: test2();
			break;
		case 3: test3();
			break;
		case 4: test4();
			break;
		default:
			break;
	}

	return 0;
}
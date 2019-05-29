#include "test/test.h"
#include <iostream>
#include "boost/bind/bind.hpp"
#include "boost/current_function.hpp"
#include "hiredis_wrapper/wrapper/redis_wrapper.hpp"
#include "slience/base/newonce_pool.h"
using namespace std;

void redis_expire(RedisConnection& conn) {
	cout << conn.expire("xiao", 4) << endl;
}

void redis_del(RedisConnection& conn) {
	cout << conn.del("xiao2") << endl;
	
	std::vector<const char*> v;
	v.push_back("xiao3");
	v.push_back("xiao4");
	cout << conn.dels(v) << endl;
}

void redis_set(RedisConnection& conn) {
	conn.set("xiao", "wo shi ni baba");
}

void redis_setnx(RedisConnection& conn) {
	cout << conn.setnx("xiao2", "wo shi") << endl;
	cout << conn.setnx("xiao", "wo shi") << endl;
}

void redis_setex(RedisConnection& conn) {
	conn.setex("xiao2", "ene en", 10);
}

void redis_get(RedisConnection& conn) {
	std::string value;
	conn.get("xiao", value);
	cout << value << endl;
}

void redis_incrby(RedisConnection& conn) {
	conn.incrby("xiao4", 3);
}

void redis_decrby(RedisConnection& conn) {
	int v = 0;
	conn.decrby("xiao4", 1, v);
	cout << v << endl;
}

void redis_strlen(RedisConnection& conn) {
	cout << conn.strlen("xiao") << endl;
}

void redis_append(RedisConnection& conn) {
	cout << conn.append("xiao", "new handle") << endl;
}

void redis_setrange(RedisConnection& conn) {
	cout << conn.setrange("xiao", 0, "ahahahah");
}

void redis_getrange(RedisConnection& conn) {
	std::string v;
	conn.getrange("xiao", 0, -1, v);
	cout << v << endl;
}

void test() {
	try {
		RedisConnection conn = RedisPool::GetConnection("192.168.1.210", 6379);
		// redis_expire(conn);
		// redis_del(conn);
		// redis_set(conn);
		// redis_setnx(conn);
		// redis_setex(conn);
		// redis_get(conn);
		// redis_incrby(conn);
		// redis_decrby(conn);
		// redis_strlen(conn);
		// redis_append(conn);
		redis_setrange(conn);
		redis_getrange(conn);
	}
	catch (RedisException& e) {
		cout << e.What() << endl;
	}
}

void test_newonce() {
	int* i1 = base::NewOncePool<int>::Alloc();
	int* i2 = base::NewOncePool<int>::Alloc();
	cout << i1 << " " << i2 << endl;
	base::NewOncePool<int>::Dealloc(i1);
	int* i3 = base::NewOncePool<int>::Alloc();
	cout << i3 << endl;
	cout << base::NewOncePool<int>::GetAllocSize() << endl;
	cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

	struct object {
		bool b;
		object() {
			b = true;
			cout << "object()" << endl;
		}
		~object() {
			cout << "~object()" << endl;
		}
	};

	object* o1 = base::NewOncePool<object,1>::Alloc();
	object* o2 = base::NewOncePool<object,1>::Alloc();
	base::NewOncePool<object,1>::Dealloc(o1);
	object* o3 = base::NewOncePool<object,1>::Alloc();
	cout << base::NewOncePool<object, 1>::GetAllocSize() << endl;
}

int main(int argc, char* argv[]) {
	test_newonce();
	return 0;
	if (false) {
		test();
	}
	else {
		TestAppSgl::mutable_instance().Init(argc, argv);
		TestAppSgl::mutable_instance().Run();
		return 0;
	}
}
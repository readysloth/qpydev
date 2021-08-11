
def init_func(obj: memoryview, obj_info: dict):
    print(obj_info)
    for i,e in enumerate(b'hellow, how are you doink?'):
        obj[len(obj)-4000+i] = e


def finalize_func(obj: memoryview, obj_info: dict):
    print(obj)

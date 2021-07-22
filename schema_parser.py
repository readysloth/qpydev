import json
import typing as t

from pathlib import Path


def load_schema(filename: str):
    schema_path = Path(filename)

    def get_load_path(path: Path):
        return path if path.is_absolute() else schema_path.parent / path

    def load_into_schema(schema_slice: dict,
                         key: str,
                         path: Path,
                         as_json=True):
        with open(path, 'r') as f:
            if as_json:
                schema_slice[key] = json.load(f)
            else:
                schema_slice[key] = f.read()


    with open(schema_path, 'r') as f:
        init_schema = json.load(f)
        for k in filter(lambda k: type(init_schema[k]) == dict, init_schema):
            subschema_path = get_load_path(Path(init_schema[k]['schema']))
            subschema_code = get_load_path(Path(init_schema[k]['code']))
            load_into_schema(init_schema[k], 'schema', subschema_path)
            load_into_schema(init_schema[k], 'code', subschema_code, as_json=False)

    return init_schema

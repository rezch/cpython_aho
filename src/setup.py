from distutils.core import setup, Extension


setup(
    name='daho',
    version="1.0",
    ext_modules=[
        Extension(
            'daho',
            [
                'module.c',
                'aho.c'
            ],
        )
    ]
)

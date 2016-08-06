# oryol-nuklear

Nuklear UI wrapper for Oryol (https://github.com/vurtun/nuklear)

Oryol sample: https://floooh.github.com/oryol-samples/asmjs/NuklearUIDemo.html

### How to integrate into your Oryol project:

Add a new import to the fips.yml file your project:

```yaml
imports:
    oryol-nuklear:
        git: https://github.com/floooh/oryol-nuklear.git
```

Next, add a new dependency 'NKUI' to your app's CMakeLists.txt file:

```cmake
fips_begin_app(MyApp windowed)
    ...
    fips_deps(NKUI)
fips_end_app()
```

Run 'fips gen' to fetch the new dependencies and rebuild the build files:

```bash
> ./fips gen
...
```

Have a look at the the [Oryol Nuklear UI sample application](https://github.com/floooh/oryol-samples/blob/master/src/NuklearUIDemo/NuklearUIDemo.cc) for how to use the NKUI module.

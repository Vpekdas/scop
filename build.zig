const std = @import("std");

pub fn build(b: *std.Build) void {
    // By default the target is the same as the host os, but it can be ovewritten by `-Dtarget=<triplet>`.
    const target = b.standardTargetOptions(.{});

    // Specify the level of optimization for the project. By default this is `.Default`, which means `-O0 -g`
    // but can be configured with `zig build --release=<fast/small/safe>`.
    // - `fast`  -> Full optimizations, no runtime safety checks, no debug info.
    // - `small` -> Optimize for file size equivalent of `-Os`.
    // - `safe`  -> Full optimization like `fast` but runtime safety checks are kept (not really relevent since we are compiling C++)
    const optimize = b.standardOptimizeOption(.{});

    // We create a compilation module, this can be either a library or a executable depending if we
    // call `b.addExecutable` or `b.addLibrary` later with it.
    // Usually we specify `.root_source_file` but since we are compiling a C++ project the source code
    // is specified with `exe.addCSourceFiles` at a later point.
    const exe_mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
    });

    // We create an executable named `scop` which use `exe_mod`.
    const exe = b.addExecutable(.{
        .name = "scop",
        .root_module = exe_mod,
    });

    // Ask zig to link against libc++, required since we are compiling C++ code.
    exe.linkLibCpp();

    // Add C++ source files
    exe.addCSourceFiles(.{
        .files = &.{
            "src/IndexBuffer.cpp",
            "src/main.cpp",
            "src/Model.cpp",
            "src/Renderer.cpp",
            "src/Shader.cpp",
            "src/Texture.cpp",
            "src/vector.cpp",
            "src/VertexBuffer.cpp",
        },
    });

    // macos requires manually setting the system include and library paths. This is usually done by running zig with `--sysroot`
    // but lets automate it.
    if (target.result.os.tag == .macos) {
        // All those frameworks are used by SDL.
        exe.linkFramework("CoreVideo");
        exe.linkFramework("Cocoa");
        exe.linkFramework("UniformTypeIdentifiers");
        exe.linkFramework("IOKit");
        exe.linkFramework("ForceFeedback");
        exe.linkFramework("Carbon");
        exe.linkFramework("CoreAudio");
        exe.linkFramework("AudioToolbox");
        exe.linkFramework("AVFoundation");
        exe.linkFramework("Foundation");
        exe.linkFramework("GameController");
        exe.linkFramework("Metal");
        exe.linkFramework("QuartzCore");
        exe.linkFramework("CoreHaptics");

        // const sdk_path = b.run(&.{ "xcrun", "--sdk", "macosx", "--show-sdk-path" });

        // exe.addLibraryPath(.{ .cwd_relative = b.pathJoin(&.{ sdk_path, "lib" }) });
        // exe.addIncludePath(.{ .cwd_relative = b.pathJoin(&.{ sdk_path, "include" }) });
    }

    // Link against SDL3. The dependency is a fork of the SDL which replace their buildsystem with zig for easier integration.
    const sdl = b.dependency("sdl", .{
        .target = target,
        .optimize = optimize,
        .preferred_link_mode = .static,
    });
    const sdl_lib = sdl.artifact("SDL3");
    exe.linkLibrary(sdl_lib);

    // Calling `sdl.path(...)` will give an absolute path inside the dependency tree where it is located,
    // somewhere in the zig cache (`~/.cache/zig/p/sdl-<some version>-<some hash>`).
    //
    // `exe.addIncludePath` is the equivalent of adding `-I<some path>` to gcc/clang.
    exe.addIncludePath(sdl.path("include"));

    // Add glad to the compilation. Since its only one .c file we can directly add the file to the executable.
    const glad = b.dependency("glad", .{});

    exe.addCSourceFile(.{
        .file = glad.path("src/glad.c"),
    });
    exe.addIncludePath(glad.path("include"));

    // Install the compiled executable in `zig-out/bin`.
    b.installArtifact(exe);

    // Add a run step that can be run with `zig build run` or `zig build run -- <args>` with arguments.
    const run_cmd = b.addRunArtifact(exe);

    // Tell the run step it depends on building the project first.
    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}

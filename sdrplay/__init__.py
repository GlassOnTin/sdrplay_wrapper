# Fixed initialization to prevent circular imports
# This addresses issue #2: Cannot import sdrplay library
try:
    from . import _sdrplay
    from .sdrplay import *
    
    # Initialize device registry on import
    # This is crucial for proper operation
    try:
        initializeDeviceRegistry()
    except Exception as e:
        import sys
        print(f"Warning: Could not initialize device registry: {e}", file=sys.stderr)
except ImportError as e:
    import sys
    print(f"Error loading SDRPlay module: {e}", file=sys.stderr)
    print("Make sure the library has been built with 'cmake .. && make' in the build directory", file=sys.stderr)

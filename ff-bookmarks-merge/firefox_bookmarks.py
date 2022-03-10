import json

FOLDER_TYPE = "text/x-moz-place-container"
BOOKMARK_TYPE = "text/x-moz-place"

BOOKMARKS_MENU_ROOT = "bookmarksMenuFolder"
BOOKMARKS_TOOLBAR_ROOT = "toolbarFolder"


def merge_files(from_filename, to_filename, out_filename):
    from_fb = FirefoxBookmarks(from_filename)
    to_fb = FirefoxBookmarks(to_filename)

    to_fb.merge(from_fb)
    to_fb.write_to(out_filename)

    return True


class FirefoxBookmarks:
    def __init__(self, fname):
        self.filename = fname

        with open(fname, 'r') as f:
            self.data = json.load(f)

        self.root = self.data

    def merge(self, fb):
        for r in [BOOKMARKS_MENU_ROOT, BOOKMARKS_TOOLBAR_ROOT]:
            if not self.merge_root(fb, r):
                print(f"[FAIL] {r}")

    def merge_root(self, fb, name):
        fb_child = None
        for child in fb.data['children']:
            if child['root'] == name:
                fb_child = child

        if fb_child is None:
            return False

        for child in self.data['children']:
            if child['root'] == name:
                self.root = child
                self.merge_folder(fb_child)

        return True
            
    def merge_folder(self, fb_root):
        try:
            fb_children = fb_root['children']
        except KeyError:
            return None

        for child in fb_children:
            cname = child['title']
            old_root = self.root

            if FirefoxBookmarks.is_folder(child):
                r = self.find_root(cname)

                if r is None:
                    self.root = self.create_folder(cname)
                    self.merge_folder(child)
                else:
                    self.root = r
                    self.merge_folder(child)

            else:
                # is bookmark
                r = self.find_root(cname)
                if r is None:
                    self.create_bookmark(child)

            self.root = old_root
                
    def find_root(self, name):
        try:
            children = self.root['children']
        except KeyError:
            return None

        for child in children:
            if child['title'] == name:
                return child
        
        return None

    def create_folder(self, name):
        try:
            children = self.root['children']
        except KeyError:
            self.root['children'] = []

        f = {"title": name, "type": FOLDER_TYPE, "children": []}
        self.root['children'].append(f)

        return f

    def create_bookmark(self, root):
        try:
            children = self.root['children']
        except KeyError:
            self.root['children'] = []

        self.root['children'].append(root)

    def write_to(self, fname):
        with open(fname, 'w') as f:
            json.dump(self.data, f, indent=4)

    @staticmethod
    def is_folder(root):
        return root['type'] == FOLDER_TYPE
    
    @staticmethod
    def is_bookmark(root):
        return root['type'] == BOOKMARK_TYPE
            


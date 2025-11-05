#include "ObjCAdapterMac.h"

#import <Cocoa/Cocoa.h>

auto ObjCAdapterMac::shareContent(const QByteArray& contentByteArray,
                                  const QString& mimeType,
                                  const QString& fileNameTemplate,
                                  const QString& fileExtension) -> QString
{
    NSData *content = [NSData dataWithBytes:contentByteArray.constData() length:contentByteArray.size()];
    if (!content || contentByteArray.isEmpty()) {
        return QString("");
    }

    NSURL *tmpDirURL = [NSURL fileURLWithPath:NSTemporaryDirectory() isDirectory:YES];
    NSURL *fileURL = [[tmpDirURL URLByAppendingPathComponent:fileNameTemplate.toNSString()]
                      URLByAppendingPathExtension:fileExtension.toNSString()];

    if (!fileURL || ![content writeToURL:fileURL atomically:YES]) {
        return "Failed to write file";
    }

    dispatch_async(dispatch_get_main_queue(), ^{
        @autoreleasepool {
            // Get main app window
            NSWindow *mainWindow = [NSApp mainWindow];
            if (!mainWindow) {
                return;
            }

            NSView *contentView = [mainWindow contentView];

            // Create picker
            NSSharingServicePicker *picker = [[NSSharingServicePicker alloc] initWithItems:@[fileURL]];

            // Position in the center of the window
            NSRect rect = NSMakeRect(NSMidX(contentView.bounds), NSMidY(contentView.bounds), 1, 1);

            [picker showRelativeToRect:rect ofView:contentView preferredEdge:NSMinYEdge];
            [picker release];
        }
    });

    return {};
}

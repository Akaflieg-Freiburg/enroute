/*!
 * Copyright (C) 2020 by Johannes Zellner, johannes@zellner.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

package de.akaflieg_freiburg.enroute;

/**
 * here goes stuff which is shared between ShareActivity and the IntentLauncher classes.
 *
 * Both classes need to use the same OPEN_REQUEST_CODE.
 * Both classes need to use the same SAVE_REQUEST_CODE.
 * IntenLauncher stores the share path by setSharePath() which
 * later be retreived by ShareActivity unsing getSharePath.
 */
public class ShareUtils {

    private static int OPEN_REQUEST_CODE = 2604;
    private static int SAVE_REQUEST_CODE = 1013;
    private static String SHARE_PATH = null;

    public static int getOpenRequestCode() {
        return OPEN_REQUEST_CODE;
    }

    public static int getSaveRequestCode() {
        return SAVE_REQUEST_CODE;
    }

    /**
     * should be set by IntentLauncher.saveFile just before the intent is launched.
     */
    public static void setSharePath(String sharePath) {
        SHARE_PATH = sharePath;
    }

    /**
     * used bye ShareActivity.onActivityResult to get source path for saving file.
     */
    public static String getSharePath() {
        String sharePath = SHARE_PATH;
        SHARE_PATH = null;
        return sharePath;
    }
}

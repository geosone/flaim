//------------------------------------------------------------------------------
// Desc:	UI Tools
//
// Tabs:	3
//
//		Copyright (c) 2003,2005-2006 Novell, Inc. All Rights Reserved.
//
//		This program is free software; you can redistribute it and/or
//		modify it under the terms of version 2 of the GNU General Public
//		License as published by the Free Software Foundation.
//
//		This program is distributed in the hope that it will be useful,
//		but WITHOUT ANY WARRANTY; without even the implied warranty of
//		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//		GNU General Public License for more details.
//
//		You should have received a copy of the GNU General Public License
//		along with this program; if not, contact Novell, Inc.
//
//		To contact Novell about this file by physical or electronic mail,
//		you may find current contact information at www.novell.com
//
// $Id: UITools.java 3120 2006-01-19 13:41:12 -0700 (Thu, 19 Jan 2006) dsanders $
//------------------------------------------------------------------------------

package xedit;
import java.awt.GridBagConstraints;

/**
 * To change the template for this generated type comment go to
 * Window->Preferences->Java->Code Generation->Code and Comments
 */
public class UITools
{
	public static void buildConstraints(
		GridBagConstraints		gbc,
		int								gx,
		int								gy,
		int								gw,
		int								gh,
		int								wx,
		int								wy)
	{
		gbc.gridx = gx;
		gbc.gridy =gy;
		gbc.gridwidth = gw;
		gbc.gridheight = gh;
		gbc.weightx = wx;
		gbc.weighty = wy;
	}
}

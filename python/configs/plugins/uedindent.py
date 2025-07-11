import logging
import time
import ctypes as ct
from far2l.plugin import PluginBase
import far2lc


log = logging.getLogger(__name__)


class Plugin(PluginBase):
    label = "Python ED Indent"
    openFrom = ["PLUGINSMENU", "EDITOR"]

    def OpenPlugin(self, OpenFrom):
        return 0

    def ProcessEditorInput(self, Rec):
        rec = self.ffi.cast("INPUT_RECORD *", Rec)

        if (
            rec.EventType != self.ffic.KEY_EVENT
            or not rec.Event.KeyEvent.bKeyDown
            or (rec.Event.KeyEvent.wVirtualKeyCode != self.ffic.VK_TAB)
            or (rec.Event.KeyEvent.dwControlKeyState & (
                self.ffic.RIGHT_ALT_PRESSED
                | self.ffic.LEFT_ALT_PRESSED
                | self.ffic.RIGHT_CTRL_PRESSED
                | self.ffic.LEFT_CTRL_PRESSED
            ))
        ):
            return 0
        indent = (rec.Event.KeyEvent.dwControlKeyState & (self.ffic.SHIFT_PRESSED)) != 0
        self.Perform(indent)
        return 1

    def Perform(self, indent):
        ei = self.ffi.new("struct EditorInfo *")
        self.info.EditorControl(self.ffic.ECTL_GETINFO, ei)

        if ei.BlockType != self.ffic.BTYPE_STREAM:
            # working only if not vertical block selection
            return 0;

        egs = self.ffi.new("struct EditorGetString *")
        egs.StringNumber = ei.BlockStartLine
        self.info.EditorControl(self.ffic.ECTL_GETSTRING, egs)
        if egs.SelEnd != -1:
            return 0

        eur = self.ffi.new("struct EditorUndoRedo *")
        eur.Command = self.ffic.EUR_BEGIN
        self.info.EditorControl(self.ffic.ECTL_UNDOREDO, eur)

        IndentStr = '\t'
        fIndentStr = self.s2f(IndentStr)

        for lno in range(ei.BlockStartLine, ei.TotalLines):
            esp = self.ffi.new("struct EditorSetPosition *")
            esp.CurLine = lno
            esp.CurPos = esp.Overtype = 0
            esp.CurTabPos = esp.TopScreenLine = esp.LeftPos = -1
            self.info.EditorControl(self.ffic.ECTL_SETPOSITION, esp)

            egs = self.ffi.new("struct EditorGetString *")
            egs.StringNumber = -1
            self.info.EditorControl(self.ffic.ECTL_GETSTRING, egs)

            if egs.SelStart == -1 or egs.SelStart == egs.SelEnd:
                # Stop when reaching the end of the text selection
                break

            if egs.SelEnd != -1:
                # if selection in line not up to end we force selection block up to line end
                # because if selection not up to end the tab replace selected text in last line
                es = self.ffi.new("struct EditorSelect *")
                es.BlockType = ei.BlockType
                es.BlockStartLine = ei.BlockStartLine
                es.BlockStartPos = 0
                es.BlockWidth = 0
                es.BlockHeight = line - ei.BlockStartLine + 1
                self.info.EditorControl(self.ffic.ECTL_SELECT, es)

            if indent:
                self.info.EditorControl(self.ffic.ECTL_INSERTTEXT, fIndentStr)
            else:
                if egs.StringText[0] == '\t':
                    n = 1
                else:
                    n = 0
                    s = self.f2s(egs.StringText)
                    while n < ei.TabSize and s[n] == ' ':
                        n += 1
                for i in range(n):
                    self.info.EditorControl(self.ffic.ECTL_DELETECHAR, self.ffi.NULL)
            
        eur = self.ffi.new("struct EditorUndoRedo *")
        eur.Command = self.ffic.EUR_END
        self.info.EditorControl(self.ffic.ECTL_UNDOREDO, eur)

        self.info.EditorControl(self.ffic.ECTL_REDRAW, self.ffi.NULL)
